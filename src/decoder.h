#ifndef DECODER_H
#define DECODER_H

#include <string>
#include "rhombicToricLattice.h"
#include "code.h"
#include <algorithm>
#include <cmath>

std::vector<bool> runToric(const int l, const int rounds,
                           const double p, const double q,
                           const std::string &sweepDirection)
{
    std::vector<bool> success = {false, false};
    Code code = Code(l, "rhombic toric", p, q);
    vint &syndrome = code.getSyndrome();
    for (int r = 0; r < rounds; ++r)
    {
        code.generateDataError();
        code.calculateSyndrome();
        if (q > 0)
        {
            code.generateMeasError();
        }
        code.sweep(sweepDirection, true);
    }
    code.generateDataError(); // Data errors = measurement errors at readout
    code.calculateSyndrome();
    // code.printUnsatisfiedStabilisers();
    for (int r = 0; r < l * l * l; ++r)
    // for (int r = 0; r < l * l; ++r)
    // for (int r = 0; r < 12 * l; ++r)
    {
        code.sweep(sweepDirection, true);
        code.calculateSyndrome();
        if (std::all_of(syndrome.begin(), syndrome.end(), [](int i) { return i == 0; }))
        {
            // std::cout << "Clean Syndrome" << std::endl;
            success = {code.checkCorrection(), true};
            break;
        }
    }
    return success;
}

std::vector<bool> runBoundaries(const int l, const int rounds,
                                const double p, const double q)
{
    std::vector<bool> success = {false, false};
    Code code = Code(l, "rhombic boundaries", p, q);
    vint &syndrome = code.getSyndrome();
    // vstr sweepDirections = {"xyz", "xz", "-xy", "yz", "xy", "-yz", "-xyz", "-xz"};
    // vstr sweepDirections = {"xyz", "xy", "yz", "xz", "-xyz", "-xy", "-yz", "-xz"};
    // vstr sweepDirections = {"yz", "-xy", "-xyz", "-xz", "xyz", "xz", "-yz", "xy"};
    // vstr sweepDirections = {"yz", "-yz", "-xy", "xy", "xyz", "-xyz", "xz", "-xz"};
    // vstr sweepDirections = {"xyz", "-xy", "-yz", "-xz", "yz", "xz", "-xyz", "xy"};
    // vstr sweepDirections = {"-xz", "xyz", "xy", "yz", "xz", "-xyz", "-xy", "-yz"};
    // vstr sweepDirections = {"xyz", "-yz", "xz", "xy", "-xz", "-xy", "-xyz", "yz"};
    // vstr sweepDirections = {"xyz", "-xy", "xz", "yz", "-xz", "-yz", "-xyz", "xy"};
    // vstr sweepDirections = {"xyz", "-xz", "-yz", "-xy", "-xyz", "xz", "yz", "xy"};
    // vstr sweepDirections = {"xyz", "-yz", "xz", "xy", "yz", "-xyz", "-xy", "-xz"};
    vstr sweepDirections = {"xyz", "-yz", "-xz", "-xy", "-xyz", "yz", "xz", "xy"};
    // std::random_shuffle(sweepDirections.begin(), sweepDirections.end());
    int sweepCount = 0;
    // int sweepLimit = (int)ceil(log(l));
    int sweepLimit = l;
    // int sweepLimit = 1;
    // int sweepLimit = pow(l, 2);
    int sweepIndex = 0;
    for (int r = 0; r < rounds; ++r)
    {
        if (sweepCount == sweepLimit)
        {
            sweepIndex = (sweepIndex + 1) % 8;
            sweepCount = 0;
        }
        code.generateDataError();
        code.calculateSyndrome();
        if (q > 0)
        {
            // std::cerr << "Generating measurement error." << std::endl; 
            code.generateMeasError();
        }
        code.sweep(sweepDirections[sweepIndex], true);
        // std::cerr << "direction=" << sweepDirections[sweepIndex] << std::endl;
        // std::cerr << "sweepIndex=" << sweepIndex << std::endl;
        // std::cerr << "sweepCount=" << sweepCount << std::endl;
        ++sweepCount;
    }
    code.generateDataError(); // Data errors = measurement errors at readout
    code.calculateSyndrome();
    // code.printUnsatisfiedStabilisers();
    // for (int r = 0; r < 12 * l; ++r)
    // for (int r = 0; r < 8 * sweepLimit * pow(l, 3); ++r)
    for (int r = 0; r < 8 * pow(l, 2); ++r)
    {
        if (sweepCount == sweepLimit)
        {
            sweepIndex = (sweepIndex + 1) % 8;
            sweepCount = 0;
        }
        code.sweep(sweepDirections[sweepIndex], true);
        code.calculateSyndrome();
        if (std::all_of(syndrome.begin(), syndrome.end(), [](int i) { return i == 0; }))
        {
            // std::cout << "Clean Syndrome" << std::endl;
            success = {code.checkCorrection(), true};
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