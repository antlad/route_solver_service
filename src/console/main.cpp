#include <solve/solver.hpp>
#include <route_solver/log.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>

namespace po = boost::program_options;
namespace bf = boost::filesystem;


//--osrm=/home/vlad/data/osrm-mow/RU-MOW.osrm --task=/home/vlad/projects/create_route/data/test1.json
int main(int argc, const char* argv[])
{
	try
	{
		std::setlocale(LC_ALL, "en_US.UTF-8");
		std::string input_file;
		std::string osrm_path;
		std::string export_folder;
		std::string export_format;
		po::options_description desc("Allowed options");
		desc.add_options()
		("task", po::value<std::string>(&input_file)->required(), "input task file")
		("osrm", po::value<std::string>(&osrm_path)->required(), "osrm path to map")
		("export_to_folder", po::value<std::string>(&export_folder), "set up export folder (for developers)")
		("export_format", po::value<std::string>(&export_format)->default_value("csv"), "export data format")
		("help", "produce help message");
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
		auto check_path = [](const auto & path)
		{
			if (!bf::exists(path))
			{
				const auto& s = (boost::format("File path %s doesn't exists") % path).str();
				BOOST_THROW_EXCEPTION(std::runtime_error(s));
			}
		};
		check_path(input_file);
		check_path(osrm_path);
		std::ifstream ifs(input_file.c_str());
		auto pbuf = ifs.rdbuf();
		// get file size using buffer's members
		auto size = pbuf->pubseekoff(0, ifs.end, ifs.in);
		pbuf->pubseekpos(0, ifs.in);
		std::string data;
		data.resize(size);
		pbuf->sgetn(&data[0], size);
		auto j = nlohmann::json::parse(data);
		rs::log::setLogLevelFromString(j["settings"]["log_level"]);
		rs::log::initializeJustConsole();
		rto::JsonParser p(j);

		if (!export_folder.empty())
		{
			check_path(export_folder);

			if (export_format != "csv")
			{
				BOOST_THROW_EXCEPTION(std::runtime_error("Other then csv is not implemented"));
			}

			p.produceInputDataCsvToFolder(export_folder);
		}

		rto::Solver s(osrm_path);
		s.solve(p.getVenchiles(), p.getZones(), p.getTasks(), p.getSpeedAprox(), p.getPartialSolutionEnabled(), p.getMultiThreaded(), p.getAbcParams());

		if (vm.count("help") || argc < 2)
		{
			std::cout << desc << std::endl;
			return 0;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << boost::diagnostic_information(e);
		return 1;
	}

	return 0;
}
