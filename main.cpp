#include <string_view>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

#include <windows.h>   // WinApi header

using Color = std::vector<int>;

struct Pixel {
    char c;
    Color color;
};

using CharMap = std::vector<std::vector<Pixel>>;

class ASCIIImage {
public:
    ASCIIImage(const std::string_view source_path) {
        read_from_file(source_path);

        m_modes = {{255, 0, 0}, {255, 255, 0}, {0, 0, 255}};
        m_mode = 0;

        if (m_char_map.empty() || m_char_map[0].empty())
            throw "Image can't have empty axis";
    }

    CharMap get_image() const {
        auto map = std::vector<std::vector<Pixel>>();

        for (int i = 0; i < m_char_map.size(); ++i) {
            map.push_back(std::vector<Pixel>(m_char_map[i].size()));
            for (int j = 0; j < m_char_map[i].size(); ++j) {
                map[i][j] = {m_char_map[i][j], get_color(m_char_map[i][j])};
            }
        }

        return map;
    }

    void change_mode() {
        m_mode = ++m_mode % m_modes.size();
    }

private:
    void read_from_file(const std::string_view source_path) {
        auto file = std::ifstream(source_path.data());
        if (!file.is_open())
            throw "Ascii file can't be opened";

        std::string s; 
        while (std::getline(file,s)) {
            m_char_map.push_back(s);
        }
    }

    std::vector<int> get_color(const char symbol) const {
        switch (symbol) {
            case '@':
                return m_modes[m_mode];
            case '*':
                return {0, 255, 0};
            default:
                return {255, 255, 255};
        }
    }

    std::vector<std::string> m_char_map;
    std::vector<std::vector<int>> m_modes;
    std::atomic<int> m_mode;
};

class ImageRenderer {
public:
    virtual void render(const ASCIIImage& image) = 0;
};

class WindowsConsoleRenderer : public ImageRenderer {
    using ConsoleColor = int;
public:
    void render(const ASCIIImage& image) override {
        auto im = image.get_image();
        HANDLE  hConsole;
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        std::system("cls");

        for (const auto& row : im) {
            for (const auto& pixel : row) {
                SetConsoleTextAttribute(hConsole, convert_colort(pixel.color));
                std::cout << pixel.c;
            }

            std::cout << std::endl;
        }
    }

private:
    ConsoleColor convert_colort(const Color& color) {
        std::map<std::vector<int>, int> color_convert = {
            {{0, 255, 0}, 2},
            {{255, 255, 0}, 3},
            {{255, 0, 0}, 4},
            {{0, 0, 255}, 5},
            {{255, 255, 255}, 15}
        };

        if (color_convert.count(color))
            return color_convert[color];
        else
            return 0;
    }
};

int main() {
    using namespace std::chrono_literals;
    auto image = ASCIIImage("C:\\Dev\\ASCII-light\\input.txt");

    auto t = std::thread([&image]() {

        std::cout << "Thread started...\n";
            int n;
            while (std::cin >> n) {
                image.change_mode();
            } 
        std::cout << "Thread exit...\n";
    });


    auto renderer = WindowsConsoleRenderer();

    for (;;) {
        renderer.render(image);
        std::this_thread::sleep_for(400ms);
    }

    t.join();
    return 0;
}