#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define BROWN "\033[33m"

// Функция для создания входного файла
void createInputFile(const string& filename, const vector<string>& lines) {
    ofstream outfile(filename);
    for (const auto& line : lines) {
        outfile << line << endl;
    }
    outfile.close();
}

// Функция для чтения содержимого файла в вектор строк
vector<string> readFile(const string& filename) {
    vector<string> lines;
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();
    return lines;
}

// Функция для сравнения двух векторов строк
bool compareVectors(const vector<string>& v1, const vector<string>& v2) {
    return v1 == v2;
}

// Функция для отображения содержимого файла
void displayFileContent(const string& filename) {
    if (!fs::exists(filename)) {
        cout << RED << "Файл не найден: " << filename << RESET << endl;
        return;
    }

    cout << BROWN << "Содержимое " << filename << ":" << RESET << endl;
    auto lines = readFile(filename);
    for (const auto& line : lines) {
        cout << "  " << line << endl;
    }
}

// Функция для выполнения теста
void runTest(const string& test_name, const string& input_file, const unordered_map<string, vector<string>>& expected_output, const string& output_dir) {
    cout << BLUE << "Выполняется тест: " << test_name << RESET << endl;

    // Отображение содержимого входного файла
    displayFileContent(input_file);

    // Очистка выходной директории
    for (const auto& entry : fs::directory_iterator(output_dir)) {
        fs::remove(entry);
    }

    // Запуск основной программы
    int result_code = system("/home/dervvilang/contacts_project/parallel_processing");
    if (result_code != 0) {
        cout << RED << "\tОшибка: Программа завершилась с кодом " << result_code << RESET << endl;
        return;
    }

    // Проверка результатов
    bool passed = true;
    for (const auto& [filename, expected_lines] : expected_output) {
        string filepath = output_dir + "/" + filename;

        // Отображение содержимого выходного файла
        displayFileContent(filepath);

        if (!fs::exists(filepath)) {
            if (!expected_lines.empty()) {
                cout << RED << "\tОшибка: Отсутствует файл " << filename << RESET << endl;
                passed = false;
            } else {
                // Создание пустого файла, если ожидается пустой
                ofstream empty_file(filepath);
                empty_file.close();
            }
            continue;
        }
        auto actual_lines = readFile(filepath);
        if (!compareVectors(actual_lines, expected_lines)) {
            cout << RED << "\tОшибка: Несоответствие содержимого в файле " << filename << RESET << endl;
            passed = false;
        }
    }

    if (passed) {
        cout << GREEN << "\tТест успешно пройден!" << RESET << endl;
    } else {
        cout << RED << "\tТест провален." << RESET << endl;
    }
}

int main() {
    const string input_file = "contacts.txt";
    const string output_dir = "results";

    // Создание выходной директории, если она не существует
    if (!fs::exists(output_dir)) {
        fs::create_directory(output_dir);
    }

    // Набор тестов
    vector<pair<string, unordered_map<string, vector<string>>>> tests = {
        {
            "Test 1: Basic Functionality",
            {
                {"И.txt", {"Иванов;Иван;Иванович;+79161234567"}},
                {"П.txt", {"Петров;Петр;Петрович;+79261234568"}},
                {"С.txt", {"Сидоров;Сидор;Сидорович;+79361234569"}},
                {"Е.txt", {"Егоров;Егор;Егорович;+79231234564"}}
            }
        },
        {
            "Test 2: Mixed Alphabets",
            {
                {"И.txt", {"Иванов;Иван;Иванович;+79161234567"}},
                {"S.txt", {"Smith;John;Michael;+12345678901"}},
                {"П.txt", {"Петров;Петр;Петрович;+79261234568"}},
                {"T.txt", {"Taylor;Emma;Olivia;+14253647890"}}
            }
        },
        {
            "Test 3: Duplicate Handling",
            {
                {"И.txt", {
                    "Иванов;Иван;Иванович;+79161234567",
                    "Исаев;Иван;Иванович;+79161234569",
                    "Ильин;Илья;Игоревич;+79161234568"
                }}
            }
        },
        {
            "Test 4: Empty Input",
            {
                // Ожидается отсутствие выходных файлов
            }
        }
    };

    while (true) {
        // Вывод меню
        cout << "Выберите тест для выполнения:" << endl;
        for (size_t i = 0; i < tests.size(); ++i) {
            cout << i + 1 << ". " << tests[i].first << endl;
        }
        cout << "0. Выход" << endl;

        // Чтение выбора пользователя
        int choice;
        cin >> choice;

        if (choice == 0) {
            break; // Завершение программы
        }

        if (choice < 1 || choice > tests.size()) {
            cout << RED << "Неверный выбор. Повторите попытку." << RESET << endl;
            continue;
        }

        // Выполнение выбранного теста
        auto& [test_name, expected_output] = tests[choice - 1];
        
        // Создание входного файла на основе тестовых данных
        vector<string> input_lines;
        for (const auto& [_, lines] : expected_output) {
            input_lines.insert(input_lines.end(), lines.begin(), lines.end());
        }
        createInputFile(input_file, input_lines);

        // Выполнение теста
        runTest(test_name, input_file, expected_output, output_dir);
    }

    return 0;
}
