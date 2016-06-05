#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <random>
#include <libgost15/libgost15.h>

const auto defaultDuration = std::chrono::duration<double, std::milli>(2000.);

enum units_t {
    kilobitsPerSecond
};


static void generateRandomBytes(uint8_t *bytes, size_t numberOfBytes) {
    std::random_device device_;
    std::mt19937 engine_(device_());
    std::uniform_int_distribution<int> distribution_(0x00, 0xff);
    auto generator_ = std::bind(distribution_, engine_);

    std::generate_n(bytes, numberOfBytes, generator_);
};


static std::string reportPerformance(std::string operation, std::string performance, bool isInProgress = false) {
    std::string result_ = std::string(80, ' ');

    if (!isInProgress) {
        std::copy(operation.begin(), operation.end(), result_.begin() + 3);
        std::copy(performance.begin(), performance.end(), result_.begin() + 55);
        result_[79] = '\n';
    }
    else {
        result_[1] = '.';
        std::copy(operation.begin(), operation.end(), result_.begin() + 3);
        result_[79] = '\r';
    }

    return result_;
}


static std::string toHumanReadable(double performance, enum units_t units) {
    std::ostringstream stream_;

    switch (units) {
        case kilobitsPerSecond: {
            stream_ << std::fixed;
            stream_ << std::setprecision(4);

            if (performance >= 1100.) {
                stream_ << performance / 1000;
                stream_ << " ";
                stream_ << "MB/s";
            }
            else {
                stream_ << performance;
                stream_ << " ";
                stream_ << "kB/s";
            }
        }
            break;
        default:
            break;
    }

    return stream_.str();
}


void benchmarkEncryption(std::chrono::duration<double, std::milli> minimumDuration) {
    std::string operation_ = "Block encryption";
    std::chrono::duration<double, std::milli> duration_(.0);
    double kBPerSecond_ = .0;

    /* Resources allocation. */
    uint8_t *roundKeys_ = new uint8_t[BlockLengthInBytes * NumberOfRounds];
    uint8_t *block_ = new uint8_t[BlockLengthInBytes];

    /* Initialisation. */
    generateRandomBytes(roundKeys_, sizeof roundKeys_);
    generateRandomBytes(block_, sizeof block_);

    /* Measurement-in-progress output. */
    std::cout << reportPerformance(operation_, "", true);

    /* Measurement cycle. */
    for (size_t iterations_ = 1; duration_ < minimumDuration; iterations_ *= 2) {
        auto startedAt_ = std::chrono::high_resolution_clock::now();

        for (size_t iterationIndex_ = 0; iterationIndex_ < iterations_; ++iterationIndex_) {
            encryptBlockWithGost15(roundKeys_, block_);
        }

        auto finishedAt_ = std::chrono::high_resolution_clock::now();
        duration_ = finishedAt_ - startedAt_;
        kBPerSecond_ = (iterations_ * BlockLengthInBytes) / (duration_.count());
    }

    /* Result output. */
    std::cout << reportPerformance(operation_, toHumanReadable(kBPerSecond_, kilobitsPerSecond));

    /* Resources releasing. */
    delete[] roundKeys_;
    delete[] block_;
}


void benchmarkDecryption(std::chrono::duration<double, std::milli> minimumDuration) {
    std::string operation_ = "Block decryption";
    std::chrono::duration<double, std::milli> duration_(.0);
    double kBPerSecond_ = .0;

    /* Resources allocation. */
    uint8_t *roundKeys_ = new uint8_t[BlockLengthInBytes * NumberOfRounds];
    uint8_t *block_ = new uint8_t[BlockLengthInBytes];

    /* Initialisation. */
    generateRandomBytes(roundKeys_, sizeof roundKeys_);
    generateRandomBytes(block_, sizeof block_);

    /* Measurement-in-progress output. */
    std::cout << reportPerformance(operation_, "", true);

    /* Measurement cycle. */
    for (size_t iterations_ = 1; duration_ < minimumDuration; iterations_ *= 2) {
        auto startedAt_ = std::chrono::high_resolution_clock::now();

        for (size_t iterationIndex_ = 0; iterationIndex_ < iterations_; ++iterationIndex_) {
            encryptBlockWithGost15(roundKeys_, block_);
        }

        auto finishedAt_ = std::chrono::high_resolution_clock::now();
        duration_ = finishedAt_ - startedAt_;
        kBPerSecond_ = (iterations_ * BlockLengthInBytes) / (duration_.count());
    }

    /* Result output. */
    std::cout << reportPerformance(operation_, toHumanReadable(kBPerSecond_, kilobitsPerSecond));

    /* Resources releasing. */
    delete[] roundKeys_;
    delete[] block_;
}


int main() {
    std::cout << "  ----------------------------------------------------------------------------  " << std::endl;
    std::cout << "   libgost15 operation                                 performance              " << std::endl;
    std::cout << "  ----------------------------------------------------------------------------  " << std::endl;

    benchmarkEncryption(defaultDuration);
    benchmarkDecryption(defaultDuration);

    std::cout << "  ----------------------------------------------------------------------------  " << std::endl;
    return 0;
}