#ifndef ENRICO_SHIFT_DRIVER_H
#define ENRICO_SHIFT_DRIVER_H

#include "enrico/geom.h"
#include "enrico/neutronics_driver.h"

#include <pugixml.hpp>

#include "Geometria/core/Geometry.hh"         // for Geometry
#include "Omnibus/driver/Multiphysics_Driver.hh" // for MultiPhysics_Driver
#include "Shift/mc_physics/SCE_Physics.hh"
#include "Teuchos_ParameterList.hpp" // for ParameterList
#include "Teuchos_RCP.hpp"           // for RCP

#include <memory> // for shared_ptr
#include <unordered_map>
#include <vector>

namespace enrico {

class ShiftDriver : public NeutronicsDriver {
public:
  // Types, aliases
  using cell_type = geometria::Geometry::cell_type;

  // Constructor
  ShiftDriver(MPI_Comm comm, pugi::xml_node node);

  //////////////////////////////////////////////////////////////////////////////
  // NeutronicsDriver interface

  // get the heat source normalized to the given total power
  xt::xtensor<double, 1> heat_source(double power) const final;

  //! Get the k-effective of a run
  UncertainDouble get_k_effective() const override {
    return UncertainDouble(0., 0.);
  }

  //! Get the boron concentration from the model
  double get_boron_ppm(const std::vector<CellHandle>& fluid_cell_handles) const override {
    return 0.;
  }

  //! Set the Boron concentration in fluid-bearing cells
  //! \param fluid_cell_handles The CellHandle objects that contain fluids
  //! \param ppm Boric acid concentration in [ppm]
  //! \param B10_iso_abund The B-10 enrichment in unitless atom-fractions
  void set_boron_ppm(const std::vector<CellHandle>& fluid_cell_handles,
                     double ppm, double B10_iso_abund) const override {}

  //! Find cells corresponding to a vector of positions
  //! \param positions (x,y,z) coordinates to search for
  //! \return Handles to cells
  std::vector<CellHandle> find(const std::vector<Position>& positions) override;

  //! Set the density of the material in a cell
  //! \param handle Handle to a cell
  //! \param rho Density in [g/cm^3]
  void set_density(CellHandle handle, double rho) const override;

  //! Set the temperature of a cell
  //! \param handle Handle to a cell
  //! \param T Temperature in [K]
  void set_temperature(CellHandle handle, double T) const override;

  //! Get the density of a cell
  //! \param handle Handle to a cell
  //! \return Cell density in [g/cm^3]
  double get_density(CellHandle handle) const override;

  //! Get the temperature of a cell
  //! \param handle Handle to a cell
  //! \return Temperature in [K]
  double get_temperature(CellHandle handle) const override;

  //! Get the volume of a cell
  //! \param handle Handle to a cell
  //! \return Volume in [cm^3]
  double get_volume(CellHandle handle) const override;

  //! Detemrine whether a cell contains fissionable nuclides
  //! \param handle Handle to a cell
  //! \return Whether the cell contains fissionable nuclides
  bool is_fissionable(CellHandle handle) const override;

  std::size_t n_cells() const override { return num_cells_; }

  //! Create energy production tallies
  void create_tallies() override;

  std::string cell_label(CellHandle handle) const override;

  gsl::index cell_index(CellHandle cell) const override;

  //////////////////////////////////////////////////////////////////////////////
  // Driver interface

  //! Initialization required in each Picard iteration
  void init_step() final;

  //! Runs Shift for one Picard iteration
  void solve_step() final;

private:
  // Data members
  std::shared_ptr<geometria::Geometry> geometry_;        //!< Core model
  std::shared_ptr<omnibus::Multiphysics_Driver> driver_; //!< Multiphysics driver

  Teuchos::RCP<Teuchos::ParameterList> plist_; //!< parameter list from input file

  std::vector<cell_type> cells_;                         //!< Shift cells
  std::unordered_map<cell_type, CellHandle> cell_index_; //!< Map cells to handles
  int num_cells_; //!< Total number of Shift cells (not size of cells_)
};

} // end namespace enrico

#endif // ENRICO_SHIFT_DRIVER_H
