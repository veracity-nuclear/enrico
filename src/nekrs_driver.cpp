#include "enrico/nekrs_driver.h"
#include "enrico/error.h"
#include "iapws/iapws.h"
#include "fldFile.hpp"
#include "nekrs.hpp"
#include "nekInterfaceAdapter.hpp"

#include <gsl-lite/gsl-lite.hpp>

// Generated by ENRICO's CMakeLists.  Defines NEKRS_HOME as CMAKE_INSTALL_PREFIX
#include "nekrs_home.h"

#include <algorithm>
#include <dlfcn.h>

namespace enrico {
NekRSDriver::NekRSDriver(MPI_Comm comm, pugi::xml_node node)
  : HeatFluidsDriver(comm, node)
{
  timer_driver_setup.start();
  if (active()) {
    // Force NEKRS_HOME
    std::stringstream msg;
    msg << "Setting env variable NEKRS_HOME=" << NEKRS_HOME;
    comm_.message(msg.str());
    err_chk(setenv("NEKRS_HOME", NEKRS_HOME, 1) == 0,
            "Could not set env variable NEKRS_HOME");

    if (node.child("output_heat_source")) {
      output_heat_source_ = node.child("output_heat_source").text().as_bool();
    }

    host_.setup({
      {"mode", "Serial"}
    });

    // commg_in and comm_in should be the same when there is only a single nekRS session.
    setup_file_ = node.child_value("casename");
    nekrs::setup(comm /* commg_in */,
                 comm /* comm_in */,
                 0 /* buildOnly */,
                 0 /* sizeTarget */,
                 0 /* ciMode */,
                 setup_file_ /* _setupFile */,
                 "" /* backend_ */,
                 "" /* _deviceId */,
		 1 /* nSessions */,
		 0 /* sessionID */,
                 0 /* debug */);

    nrs_ptr_ = reinterpret_cast<nrs_t*>(nekrs::nrsPtr());
    open_lib_udf();

    // Check that we're running a CHT simulation.
    err_chk(nrs_ptr_->cht == 1,
            "NekRS simulation is not setup for conjugate-heat transfer (CHT).  "
            "ENRICO must be run with a CHT simulation.");

    mesh_t *mesh = nrs_ptr_->cds->mesh[0];
    n_local_elem_ = mesh->Nelements;
    poly_deg_ = mesh->N;
    n_gll_ = mesh->Np;

    std::size_t n = n_local_elem_;
    MPI_Allreduce(
      &n, &n_global_elem_, 1, get_mpi_type<std::size_t>(), MPI_SUM, comm_.comm);

    x_ = mesh->x;
    y_ = mesh->y;
    z_ = mesh->z;
 
    element_info_ = mesh->elementInfo;

    auto cds = nrs_ptr_->cds;

    // Copy rho_cp_ from the device to host
    rho_cp_.resize(mesh->Nelements * mesh->Np);
    occa::memory o_rho = cds->o_rho;
    o_rho.copyTo(rho_cp_.data(), mesh->Nlocal * sizeof(dfloat)); 

    temperature_ = cds->S;

    // Copy lumped mass matrix from device to host
    mass_matrix_.resize(mesh->Nelements * mesh->Np);
    occa::memory o_LMM = mesh->o_LMM;
    o_LMM.copyTo(mass_matrix_.data(), mesh->Nlocal * sizeof(dfloat));
  }

#ifdef _OPENMP
#pragma omp parallel default(none) shared(num_threads)
#pragma omp single
  num_threads = omp_get_num_threads();
#endif

  timer_driver_setup.stop();
}

void NekRSDriver::init_step()
{
  timer_init_step.start();
  auto min = std::min_element(localq_->cbegin(), localq_->cend());
  auto max = std::max_element(localq_->cbegin(), localq_->cend());
  timer_init_step.stop();
}

void NekRSDriver::solve_step()
{
  timer_solve_step.start();
  const int runtime_stat_freq = 500;
  auto elapsed_time = MPI_Wtime();
  tstep_ = 0;
  time_ = nekrs::startTime();

  nekrs::udfExecuteStep(time_, tstep_, 0);

  auto last_step = nekrs::lastStep(time_, tstep_, elapsed_time);
  double elapsedStepSum = 0;

  if (!last_step) {
    std::stringstream msg;
    if (nekrs::endTime() > nekrs::startTime()) {
      msg << "timestepping to time " << nekrs::endTime() << " ...";
    } else {
      msg << "timestepping for " << nekrs::numSteps() << " steps ...";
    }
    comm_.message(msg.str());
  }

  while (!last_step) {
    if (comm_.active())
      comm_.Barrier();
    const double timeStartStep = MPI_Wtime();
    elapsed_time += (MPI_Wtime() - elapsed_time);
    ++tstep_;
    last_step = nekrs::lastStep(time_, tstep_, elapsed_time);

    double dt;
    if (last_step && nekrs::endTime() > 0)
      dt = nekrs::endTime() - time_;
    else
      dt = nekrs::dt(tstep_);

    int outputStep = nekrs::outputStep(time_ + dt, tstep_);
    if (nekrs::writeInterval() == 0) outputStep = 0;
    if (last_step) outputStep = 1;
    if (nekrs::writeInterval() < 0) outputStep = 0;
    nekrs::outputStep(outputStep);    

    nekrs::initStep(time_, dt, tstep_);

    int corrector = 1;
    bool converged = false;
    do {
      converged = nekrs::runStep(corrector++);
    } while (!converged);

    time_ = nekrs::finishStep();

    comm_.Barrier();
    const double elapsedStep = MPI_Wtime() - timeStartStep;
    elapsedStepSum += elapsedStep;
    nekrs::updateTimer("elapsedStep", elapsedStep);
    nekrs::updateTimer("elapsedStepSum", elapsedStepSum);
    nekrs::updateTimer("elapsed", elapsedStepSum); 

    if (nekrs::printInfoFreq()) {
      if (tstep_ % nekrs::printInfoFreq() == 0)
        nekrs::printInfo(time_, tstep_, true, false);
    }
    
    if (tstep_ % runtime_stat_freq == 0 || last_step)
      nekrs::printRuntimeStatistics(tstep_);
  }
  timer_solve_step.stop();
}

void NekRSDriver::write_step(int timestep, int iteration)
{
  timer_write_step.start();
  nekrs::outfld(time_, timestep);
  int FP64 = 1; 
  if (output_heat_source_) {
    comm_.message("Writing heat source to .fld file");
    occa::memory o_localq =
      host_.wrapMemory<double>(localq_->data(), localq_->size());
    writeFld("qsc", time_, 1, 0, FP64, &nrs_ptr_->o_U, &nrs_ptr_->o_P, &o_localq, 1);
  }
  timer_write_step.stop();
}

Position NekRSDriver::centroid_at(int32_t local_elem) const
{
  Expects(local_elem < n_local_elem());
  Position c{0., 0., 0.};
  double mass = 0.;
  for (gsl::index i = 0; i < n_gll_; ++i) {
    auto idx = local_elem * n_gll_ + i;
    c.x += x_[idx] * mass_matrix_[idx];
    c.y += y_[idx] * mass_matrix_[idx];
    c.z += z_[idx] * mass_matrix_[idx];
    mass += mass_matrix_[idx];
  }
  c.x /= mass;
  c.y /= mass;
  c.z /= mass;
  return c;
}

std::vector<Position> NekRSDriver::centroid() const
{
  std::vector<Position> c(n_local_elem());
  for (int32_t i = 0; i < n_local_elem(); ++i) {
    c[i] = this->centroid_at(i);
  }
  return c;
}

double NekRSDriver::volume_at(int32_t local_elem) const
{
  Expects(local_elem < n_local_elem());
  double v = 0.;
  for (int32_t i = 0; i < n_gll_; ++i) {
    v += mass_matrix_[local_elem * n_gll_ + i];
  }
  return v;
}

std::vector<double> NekRSDriver::volume() const
{
  std::vector<double> v(n_local_elem());
  for (int32_t i = 0; i < n_local_elem(); ++i) {
    v[i] = this->volume_at(i);
  }
  return v;
}

double NekRSDriver::temperature_at(int32_t local_elem) const
{
//  comm_.message("at the beginning of function temperature_at()");
  Expects(local_elem < n_local_elem());

  double sum0 = 0.;
  double sum1 = 0.;
  for (int32_t i = 0; i < n_gll_; ++i) {
    auto idx = local_elem * n_gll_ + i;
    sum0 += rho_cp_[idx] * temperature_[idx];
    sum1 += rho_cp_[idx];
  }
  return sum0 / sum1;
}

std::vector<double> NekRSDriver::temperature() const
{
  std::vector<double> t(n_local_elem());
  for (int32_t i = 0; i < n_local_elem(); ++i) {
    t[i] = this->temperature_at(i);
  }
  
  return t;
}

std::vector<double> NekRSDriver::density() const
{
  nek::copyToNek(time_, tstep_);
  std::vector<double> local_densities(n_local_elem());

  for (int32_t i = 0; i < n_local_elem(); ++i) {
    if (this->in_fluid_at(i) == 1) {
      auto T = this->temperature_at(i);
      // nu1 returns specific volume in [m^3/kg]
      local_densities[i] = 1.0e-3 / iapws::nu1(pressure_bc_, T);
    } else {
      local_densities[i] = 0.0;
    }
  }

  return local_densities;
}

int NekRSDriver::in_fluid_at(int32_t local_elem) const
{
  Expects(local_elem < n_local_elem());
  return element_info_[local_elem] == 1 ? 0 : 1;
}

std::vector<int> NekRSDriver::fluid_mask() const
{
  std::vector<int> mask(n_local_elem());
  for (int32_t i = 0; i < n_local_elem(); ++i) {
    mask[i] = in_fluid_at(i);
  }
  return mask;
}

int NekRSDriver::set_heat_source_at(int32_t local_elem, double heat)
{
  for (int i = 0; i < n_gll_; ++i) {
    localq_->at(local_elem * n_gll_ + i) = heat;
  }
  return 0;
}

void NekRSDriver::open_lib_udf()
{
  lib_udf_handle_ = dlopen(lib_udf_name_.c_str(), RTLD_LAZY);
  if (!lib_udf_handle_) {
    std::stringstream msg;
    msg << "dlopen error for localq in " << lib_udf_name_ << " : " << dlerror();
    throw std::runtime_error(msg.str());
  }
  void* localq_void = dlsym(lib_udf_handle_, "localq");
  if (dlerror()) {
    throw std::runtime_error("dlsym error for localq in " + lib_udf_name_);
  }
  localq_ = reinterpret_cast<std::vector<double>*>(localq_void);
}

void NekRSDriver::close_lib_udf()
{
  err_chk(dlclose(lib_udf_handle_), "dlclose error for " + lib_udf_name_);
}

NekRSDriver::~NekRSDriver()
{
  close_lib_udf();
}

}
