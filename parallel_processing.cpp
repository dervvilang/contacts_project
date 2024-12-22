#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

std::mutex file_mutex;

// Функция обработки сегмента данных
void processSegment(const vector<string>& segment, unordered_map<wstring, vector<string>>& result, double& elapsed_time) {
    auto start_time = chrono::high_resolution_clock::now(); // Начало отсчета времени

    for (const auto& line : segment) {
        if (!line.empty()) {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            wstring wline = converter.from_bytes(line);
            wchar_t first_letter = wline[0]; // Первый символ строки

            lock_guard<mutex> lock(file_mutex); // Блокировка доступа к общему ресурсу
            result[wstring(1, first_letter)].push_back(line);
        }
    }

    auto end_time = chrono::high_resolution_clock::now(); // Конец отсчета времени
    elapsed_time = chrono::duration<double>(end_time - start_time).count(); // Расчет времени выполнения
}

// Функция записи результатов в файлы
void writeResultsToFile(const unordered_map<wstring, vector<string>>& data, const string& output_dir) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;

    for (const auto& [key, lines] : data) {
        string filename = output_dir + "/" + converter.to_bytes(key) + ".txt";
        ofstream outfile(filename, ios::app); // Открытие файла в режиме добавления
        for (const auto& line : lines) {
            outfile << line << endl;
        }
    }
}

int main() {
    locale::global(locale("")); // Установка локали для поддержки UTF-8

    const string input_file = "contacts.txt";
    const string output_dir = "results";

    // Создание директории для результатов, если она отсутствует
    if (!fs::exists(output_dir)) {
        fs::create_directory(output_dir);
    }

    ifstream infile(input_file);

    if (!infile.is_open()) {
        cerr << "Ошибка: невозможно открыть входной файл " << input_file << endl;
        return 1;
    }

    // Чтение всех строк из файла
    vector<string> lines;
    string line;
    while (getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // Разделение данных на 4 сегмента
    size_t total_lines = lines.size();
    size_t segment_size = (total_lines + 3) / 4; // Округление вверх

    vector<vector<string>> segments(4);
    for (size_t i = 0; i < 4; ++i) {
        size_t start = i * segment_size;
        size_t end = min(start + segment_size, total_lines);
        segments[i] = vector<string>(lines.begin() + start, lines.begin() + end);
    }

    // Хеш-таблицы и трекеры времени для каждого потока
    unordered_map<wstring, vector<string>> hash_tables[4];
    double thread_times[4] = {0.0};

    // Запуск потоков для обработки сегментов
    vector<thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(processSegment, cref(segments[i]), ref(hash_tables[i]), ref(thread_times[i]));
    }

    // Ожидание завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Объединение всех результатов в одну хеш-таблицу
    unordered_map<wstring, vector<string>> final_result;
    for (int i = 0; i < 4; ++i) {
        for (const auto& [key, value] : hash_tables[i]) {
            final_result[key].insert(final_result[key].end(), value.begin(), value.end());
        }
    }

    // Запись результатов в файлы
    writeResultsToFile(final_result, output_dir);

    // Вывод времени выполнения каждого потока с округлением до 8 знаков после запятой
    for (int i = 0; i < 4; ++i) {
        cout << "Время выполнения потока " << i + 1 << ": " 
             << fixed << setprecision(8) << thread_times[i] << " секунд" << endl;
    }

    return 0;
}
