#ifndef DECODER_H
#define DECODER_H

#include <string>
#include "rhombicCode.h"
#include "cubicCode.h"
#include <algorithm>
#include <cmath>
#include "pcg_random.hpp"

pcg_extras::seed_seq_from<std::random_device> seed;
pcg32 rnEngine(seed);

// std::mt19937 rnEngine(time(0)); // Valgrind doesn't like pcg

std::uniform_int_distribution<int> distInt0To7(0, 7);

// std::vector<bool> runToric(const int l, const int rounds,
//                            const double p, const double q,
//                            const std::string &sweepDirection,
//                            const int timeout, bool greedy,
//                            bool correlatedErrors)
// {
//     std::vector<bool> success = {false, false};
//     RhombicCode code = RhombicCode(l, p, q, false);
//     std::vector<int8_t> &syndrome = code.getSyndrome();
//     for (int r = 0; r < rounds; ++r)
//     {
//         code.generateDataError(correlatedErrors);
//         code.calculateSyndrome();
//         if (q > 0)
//         {
//             code.generateMeasError();
//         }
//         code.sweep(sweepDirection, greedy);
//     }
//     code.generateDataError(correlatedErrors); // Data errors = measurement errors at readout
//     code.calculateSyndrome();
//     // code.printUnsatisfiedStabilisers();
//     for (int r = 0; r < timeout; ++r)
//     {
//         code.sweep(sweepDirection, greedy);
//         code.calculateSyndrome();
//         if (std::all_of(syndrome.begin(), syndrome.end(), [](int i) { return i == 0; }))
//         {
//             // std::cout << "Clean Syndrome" << std::endl;
//             success = {code.checkCorrection(), true};
//             break;
//         }
//     }
//     return success;
// }

std::vector<bool> oneRun(const int l, const int rounds,
                                const double p, const double q,
                                const int sweepLimit,
                                const std::string sweepSchedule,
                                const int timeout,
                                const std::string latticeType,
                                bool greedy,
                                bool correlatedErrors, 
                                const int sweepRate)
{
    std::vector<bool> success = {false, false};
    std::unique_ptr<Code> code;
    if (latticeType == "rhombic_boundaries")
    {
        code = std::make_unique<RhombicCode>(l, p, q, true, sweepRate);
    }
    else if (latticeType == "cubic_boundaries")
    {
        code = std::make_unique<CubicCode>(l, p, q, true, sweepRate);
    }
    else if (latticeType == "rhombic_toric")
    {
        code = std::make_unique<RhombicCode>(l, p, q, false, sweepRate);
    }
    else if (latticeType == "cubic_toric")
    {
        code = std::make_unique<CubicCode>(l, p, q, false, sweepRate);
    }
    if (correlatedErrors)
    {
        code->buildCorrelatedIndices();
    }
    std::vector<int8_t> &syndrome = code->getSyndrome();
    vstr sweepDirections = {"xyz", "xy", "xz", "yz", "-xyz", "-xy", "-xz", "-yz"}; // Used by random schedule
    bool randomSchedule = false;
    int sweepIndex = 0;
    int sweepCount = 0;
    if (sweepSchedule == "rotating_XZ")
    {
        sweepDirections = {"xyz", "xy", "-xz", "yz", "xz", "-yz", "-xyz", "-xy"};
    }
    else if (sweepSchedule == "alternating_XZ")
    {
        sweepDirections = {"xyz", "-xz", "-yz", "-xy", "-xyz", "xz", "yz", "xy"};
    }
    else if (sweepSchedule == "rotating_YZ")
    {
        sweepDirections = {"xyz", "xy", "-yz", "xz", "yz", "-xz", "-xyz", "-xy"};
    }
    else if (sweepSchedule == "alternating_YZ")
    {
        sweepDirections = {"xyz", "-yz", "-xz", "-xy", "-xyz", "yz", "xz", "xy"};
    }
    else if (sweepSchedule == "rotating_XY")
    {
        sweepDirections = {"xyz", "yz", "-xy", "xz", "xy", "-xz", "-xyz", "-yz"};
    }
    else if (sweepSchedule == "alternating_XY")
    {
        sweepDirections = {"xyz", "-xy", "-xz", "-yz", "-xyz", "xy", "xz", "yz"};
    }
    else if (sweepSchedule == "random")
    {
        randomSchedule = true;
        sweepIndex = distInt0To7(rnEngine);
    }
    else if (sweepSchedule == "const")
    {
        sweepDirections = {"-xyz"};
    }
    else if (sweepSchedule == "pm_XYZ")
    {
        sweepDirections = {"-xyz", "xyz"};
    }
    else if (sweepSchedule == "four_directions")
    {
        sweepDirections = {"xyz", "xy", "-xz", "yz"};
    }
    else
    {
        throw std::invalid_argument("Invalid sweep schedule.");
    }
    int numberOfDirections = sweepDirections.size();
    // std::cerr << "No. of sweep dirs: " << numberOfDirections << std::endl;
    for (int r = 0; r < rounds; ++r)
    {
        if (sweepCount == sweepLimit)
        {
            if (randomSchedule)
            {
                sweepIndex = distInt0To7(rnEngine);
            }
            else
            {
                sweepIndex = (sweepIndex + 1) % numberOfDirections;
            }
            sweepCount = 0;
        }
        code->generateDataError(correlatedErrors);
        code->calculateSyndrome();
        if (q > 0)
        {
            // std::cerr << "Generating measurement error." << std::endl;
            code->generateMeasError();
        }
        for (int i = 0; i < sweepRate; ++i)
        {
            code->sweep(sweepDirections[sweepIndex], greedy);
        }
        // std::cerr << "direction=" << sweepDirections[sweepIndex] << std::endl;
        // std::cerr << "sweepIndex=" << sweepIndex << std::endl;
        // std::cerr << "sweepCount=" << sweepCount << std::endl;
        ++sweepCount;
    }
    code->generateDataError(correlatedErrors); // Data errors = measurement errors at readout
    code->calculateSyndrome();
    // code.printUnsatisfiedStabilisers();
    for (int r = 0; r < timeout; ++r)
    {
        if (sweepCount == l)
        {
            if (randomSchedule)
            {
                sweepIndex = distInt0To7(rnEngine);
            }
            else
            {
                sweepIndex = (sweepIndex + 1) % numberOfDirections;
            }
            sweepCount = 0;
        }
        code->sweep(sweepDirections[sweepIndex], greedy);
        code->calculateSyndrome();
        if (std::all_of(syndrome.begin(), syndrome.end(), [](int i) { return i == 0; }))
        {
            // std::cout << "Clean Syndrome" << std::endl;
            success = {code->checkCorrection(), true};
            break;
        }
        // std::cerr << "r=" << r << std::endl;
        // std::cerr << "direction=" << sweepDirections[sweepIndex] << std::endl;
        ++sweepCount;
    }

    // Testing
    // std::cerr << "Error:" << std::endl;
    // code.printError();
    // std::cerr << "Unsatisfied stabilizers:" << std::endl;
    // code.printUnsatisfiedStabilisers();

    return success;
}

#endif