#include <unistd.h>
#include <GrabData.hpp>

int main (int argc, char **argv) {

    if (argc != 3) {

        // error
        std::cout << "Usage: ./parser <log_file> <output_file>";

        return -1;

    }

    // read the input filenames
    std::string input_file(argv[1]);
    std::string output_file(argv[2]);

    // std::cout << "Input file: " << input_file << "\n";
    // std::cout << "Output file: " << output_file << "\n";

    std::string carmen_home(getenv("CARMEN_HOME"));
    std::string config_filename = 3 < argc ? std::string(argv[3]) : carmen_home + "/src/hypergraphsclam/config/parser_config.txt";

    // create a grab data object
    hyper::GrabData gd;

    // configure it
    gd.Configure(config_filename);

    // try to process the log file
    if (gd.ParseLogFile(input_file)) {

        // build the hyper graph
        gd.BuildHyperGraph();

        // save the hyper graph
        gd.SaveHyperGraph(output_file);

        // save extra info
        gd.SaveEstimates();

    } else {

        std::cout << "Error! Could not parse the log file!\n";

    }

    // close everything
    gd.Clear();

    // clearing?
    std::cout << "Cleared!\n";

    return 0;


}
