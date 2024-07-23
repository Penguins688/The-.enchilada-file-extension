#include <sndfile.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>

const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

std::string base64_encode(const std::vector<uint8_t>& data) {
    std::string encoded;
    size_t i = 0;
    size_t data_size = data.size();

    while (i < data_size) {
        uint32_t octet_a = i < data_size ? data[i++] : 0;
        uint32_t octet_b = i < data_size ? data[i++] : 0;
        uint32_t octet_c = i < data_size ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded.push_back(base64_chars[(triple >> 18) & 0x3F]);
        encoded.push_back(base64_chars[(triple >> 12) & 0x3F]);
        encoded.push_back(base64_chars[(triple >> 6) & 0x3F]);
        encoded.push_back(base64_chars[triple & 0x3F]);
    }

    size_t mod = data_size % 3;
    if (mod > 0) {
        encoded[encoded.size() - 1] = '=';
        if (mod == 1) {
            encoded[encoded.size() - 2] = '=';
        }
    }

    return encoded;
}

int main() {
    std::string file_path;
    std::cout << "Enter file path: ";
    std::cin >> file_path;

    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (!infile) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    std::vector<short> audioData(sfinfo.frames * sfinfo.channels);
    sf_count_t num_frames = sf_read_short(infile, audioData.data(), sfinfo.frames * sfinfo.channels);
    if (num_frames != sfinfo.frames * sfinfo.channels) {
        std::cerr << "Error: could not read all frames." << std::endl;
        sf_close(infile);
        return 1;
    }
    sf_close(infile);

    std::vector<uint8_t> rawBinary(reinterpret_cast<uint8_t*>(audioData.data()), reinterpret_cast<uint8_t*>(audioData.data()) + audioData.size() * sizeof(short));

    std::string encoded = base64_encode(rawBinary);

    std::ofstream outFile("sample.enchilada", std::ios::binary);
    outFile << encoded;
    if (!outFile) {
        std::cerr << "Error: could not write data to file." << std::endl;
        return 1;
    }
    outFile.close();

    std::cout << "File written successfully." << std::endl;

    return 0;
}