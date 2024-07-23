#include <fstream>
#include <string>
#include <cstdint>
#include <iostream>
#include <vector>
#include <sndfile.h>
#include <SDL.h>
#include <iterator>
#include <cctype>

const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"
                                 "0123456789+/";

std::vector<uint8_t> base64_decode(const std::string &encoded) {
    std::vector<uint8_t> decoded;
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (isspace(c)) {
            continue;
        }
        if (c == '=') {
            break;
        }
        val = (val << 6) + base64_chars.find(c);
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return decoded;
}

int main() {
    std::string file_path;
    std::cout << "Enter file path: ";
    std::cin >> file_path;

    std::string encodedData;
    std::ifstream inFile(file_path, std::ios::binary);
    if (inFile) {
        inFile.seekg(0, std::ios::end);
        encodedData.reserve(inFile.tellg());
        inFile.seekg(0, std::ios::beg);
        encodedData.assign((std::istream_iterator<char>(inFile)), std::istream_iterator<char>());
        inFile.close();
    } else {
        std::cerr << "Error: could not open file" << std::endl;
        return 1;
    }

    std::vector<uint8_t> decodedData = base64_decode(encodedData);
    std::vector<short> audioData(reinterpret_cast<short*>(decodedData.data()), reinterpret_cast<short*>(decodedData.data() + decodedData.size()));

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error: could not initialize SDL" << std::endl;
        return 1;
    }

    SDL_AudioSpec wavSpec;
    wavSpec.freq = 44100;
    wavSpec.format = AUDIO_S16SYS;
    wavSpec.channels = 2;
    wavSpec.samples = 4096;
    wavSpec.callback = nullptr;

    if (SDL_OpenAudio(&wavSpec, nullptr) < 0) {
        std::cerr << "Error: could not open audio device" << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_QueueAudio(1, audioData.data(), audioData.size() * sizeof(short));
    SDL_PauseAudio(0);

    int duration_ms = (audioData.size() / wavSpec.channels) * 1000 / wavSpec.freq;
    SDL_Delay(duration_ms);

    SDL_CloseAudio();
    SDL_Quit();

    std::cout << "Audio played successfully" << std::endl;
    return 0;
}