#include <stdexcept>

#include "pugixml.hpp"
#include <mpi.h>

#include "Nemesis/comm/Functions.hh"
#include "Omnibus/driver/Multiphysics_Driver.hh"
#include "Omnibus/driver/Sequence_Shift.hh" // for Sequence_Shift
#include "Shift/mc_tallies/Cell_Union_Tally.hh"
#include "Teuchos_DefaultMpiComm.hpp"          // for MpiComm
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp" // for RCP, ParameterList

#include <unordered_map>

struct Position {
  // Constructors
  Position() {}
  Position(double x0, double y0, double z0)
    : x{x0}
    , y{y0}
    , z{z0}
  {}

  double x{}; //!< x-coordinate
  double y{}; //!< y-coordinate
  double z{}; //!< z-coordinate
};

int main(int argc, char* argv[])
{
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Parse enrico.xml file
  pugi::xml_document doc;
  auto result = doc.load_file("enrico.xml");
  if (!result) {
    throw std::runtime_error{"Unable to load enrico.xml file"};
  }

  // Get root element
  auto root = doc.document_element();

  // Create driver according to selections
  {
    int max_timesteps_ = 1;
    int max_picard_iter_ = 1;
    auto node = root.child("neutronics");
    // std::unique_ptr<enrico::ShiftDriver> neutronics_driver_ =
    //     std::make_unique<enrico::ShiftDriver>(MPI_COMM_WORLD, node);
    std::string filename = node.child_value("filename");
    Teuchos::RCP<Teuchos::ParameterList> plist_ = Teuchos::RCP<Teuchos::ParameterList>(
      new Teuchos::ParameterList("Omnibus_plist_root"));
    plist_->set("input_path", filename);
    auto teuchos_comm = Teuchos::MpiComm<int>(MPI_COMM_WORLD);
    std::cout << "filename = " << filename << std::endl;
    std::cout << "plist = " << std::endl;
    plist_->print(std::cout);
    std::cout << "comms = " << teuchos_comm << " " << MPI_COMM_WORLD << std::endl;
    Teuchos::updateParametersFromXmlFileAndBroadcast(
      filename, plist_.ptr(), teuchos_comm);
    auto problem = std::make_shared<omnibus::Problem>(plist_);
    std::shared_ptr<omnibus::Multiphysics_Driver> driver_ = std::make_shared<omnibus::Multiphysics_Driver>(problem);
    std::shared_ptr<geometria::Geometry> geometry_ = problem->geometry();
    // num_cells_ = geometry_->num_cells();

    std::vector<geometria::Geometry::cell_type> cells_;
    std::unordered_map<geometria::Geometry::cell_type, std::size_t> cell_index_;
    std::vector<Position> positions;
    positions.emplace_back(0.0, 0.0, 0.5);
    positions.emplace_back(0.51, 0.0, 0.5);
    positions.emplace_back(0.42, 0.0, 0.5);
    positions.emplace_back(0.5, 0.0, 0.5);
    // std::vector<std::size_t> elem_to_glob_cell_ = neutronics_driver_->find(positions);
    std::vector<std::size_t> elem_to_glob_cell_;
    elem_to_glob_cell_.reserve(positions.size());
    for (const auto& r : positions) {
      geometria::Space_Vector vec = {r.x, r.y, r.z};
      geometria::Geometry::cell_type cell = geometry_->find_cell(vec);
      if (cell_index_.find(cell) == cell_index_.end()) {
        cell_index_[cell] = cells_.size();
        cells_.push_back(cell);
      }
      auto h = cell_index_.at(cell);
      elem_to_glob_cell_.push_back(h);
    }

    // neutronics_driver_->create_tallies();
    auto tally_pl = Teuchos::sublist(plist_, "TALLY");
    auto cell_pl = Teuchos::sublist(tally_pl, "CELL");
    using Teuchos::Array;
    auto power_pl = Teuchos::sublist(cell_pl, "power");
    power_pl->set("name", "power");
    power_pl->set("normalization", 1.0);
    power_pl->set("description", std::string("power tally"));
    Array<std::string> rxns(1, "fission");
    power_pl->set("reactions", rxns);
    power_pl->set("cycles", std::string("active"));
    Array<std::string> cells(cells_.size());
    for (int cellid = 0; cellid < cells_.size(); ++cellid){
      cells[cellid] = geometry_->cell_to_label(cells_[cellid]);
    }
    Array<int> counts(cells_.size(), 1);
    power_pl->set("union_cells", cells);
    power_pl->set("union_lengths", counts);

    for (int i_timestep_ = 0; i_timestep_ < max_timesteps_; ++(i_timestep_)){
      for (int i_picard_ = 0; i_picard_ < max_picard_iter_; ++(i_picard_)){
        driver_->rebuild();
        driver_->run();
      }
    }
  } // Make sure to deconstruct the driver before calling MPI_Finalize()

  MPI_Finalize();
  return 0;
}
