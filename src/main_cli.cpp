#include "PasswordManager.h"
#include "PasswordGenerator.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#endif

// Helper to clear the input buffer
void clearInput() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

// Read line safely (handles spaces)
std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// Print entries in a formatted table
void printEntriesTable(const std::vector<PasswordEntry>& list, bool showPasswords) {
    if (list.empty()) {
        std::cout << "Записи не найдены.\n";
        return;
    }

    std::cout << "\n" << std::left 
              << std::setw(20) << "Сервис" 
              << std::setw(25) << "Логин / Email" 
              << std::setw(20) << "Пароль" 
              << std::setw(25) << "URL" 
              << "Комментарий\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto& e : list) {
        std::string pwd = showPasswords ? e.password : "********";
        std::cout << std::left 
                  << std::setw(20) << (e.serviceName.length() > 18 ? e.serviceName.substr(0, 15) + "..." : e.serviceName)
                  << std::setw(25) << (e.username.length() > 23 ? e.username.substr(0, 20) + "..." : e.username)
                  << std::setw(20) << (pwd.length() > 18 ? pwd.substr(0, 15) + "..." : pwd)
                  << std::setw(25) << (e.url.length() > 23 ? e.url.substr(0, 20) + "..." : e.url)
                  << (e.note.length() > 20 ? e.note.substr(0, 17) + "..." : e.note) << "\n";
    }
    std::cout << "\n";
}

int main() {
    // Enable UTF-8 encoding in Windows Console
#if defined(_WIN32)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "=== Менеджер Паролей C++ ===\n\n";

    std::string vaultPath = "vault.dat";
    PasswordManager pm(vaultPath);

    // Authentication flow
    if (!pm.hasVault()) {
        std::cout << "Хранилище не обнаружено. Создание нового мастер-пароля.\n";
        while (true) {
            std::string p1 = readLine("Придумайте мастер-пароль: ");
            std::string p2 = readLine("Подтвердите мастер-пароль: ");
            if (p1.empty()) {
                std::cout << "Пароль не может быть пустым!\n";
                continue;
            }
            if (p1 != p2) {
                std::cout << "Пароли не совпадают. Повторите ввод.\n";
                continue;
            }
            try {
                pm.createVault(p1);
                std::cout << "Хранилище успешно создано!\n\n";
                break;
            } catch (const std::exception& e) {
                std::cout << "Ошибка при создании хранилища: " << e.what() << "\n";
            }
        }
    } else {
        std::cout << "Вход в существующее хранилище.\n";
        int attempts = 3;
        while (attempts > 0) {
            std::string password = readLine("Введите мастер-пароль: ");
            try {
                if (pm.unlock(password)) {
                    std::cout << "Доступ разрешен!\n\n";
                    break;
                } else {
                    attempts--;
                    std::cout << "Неверный мастер-пароль. Осталось попыток: " << attempts << "\n";
                }
            } catch (const std::exception& e) {
                std::cout << "Критическая ошибка при открытии: " << e.what() << "\n";
                return 1;
            }
        }
        if (!pm.unlocked()) {
            std::cout << "Доступ заблокирован. Выход.\n";
            return 1;
        }
    }

    // Main menu loop
    bool running = true;
    while (running) {
        std::cout << "=== ГЛАВНОЕ МЕНЮ ===\n"
                  << "1. Добавить пароль\n"
                  << "2. Показать все записи\n"
                  << "3. Найти запись\n"
                  << "4. Редактировать запись\n"
                  << "5. Удалить запись\n"
                  << "6. Сгенерировать новый пароль\n"
                  << "7. Сохранить изменения и выйти\n"
                  << "Выберите действие (1-7): ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Неверный ввод. Введите число от 1 до 7.\n\n";
            continue;
        }
        clearInput(); // Consume newline

        switch (choice) {
            case 1: { // Add
                std::cout << "\n--- ДОБАВЛЕНИЕ ЗАПИСИ ---\n";
                PasswordEntry entry;
                entry.serviceName = readLine("Название сервиса (например, GitHub): ");
                if (entry.serviceName.empty()) {
                    std::cout << "Название сервиса не может быть пустым!\n\n";
                    break;
                }
                entry.username = readLine("Логин или Email: ");
                
                std::cout << "Сгенерировать пароль автоматически? (y/n): ";
                std::string autoPwd;
                std::getline(std::cin, autoPwd);
                if (autoPwd == "y" || autoPwd == "Y") {
                    try {
                        entry.password = PasswordGenerator::generate(16, true, true, true, true);
                        std::cout << "Сгенерирован пароль: " << entry.password << "\n";
                    } catch (const std::exception& e) {
                        std::cout << "Ошибка генерации, введите пароль вручную.\n";
                        entry.password = readLine("Пароль: ");
                    }
                } else {
                    entry.password = readLine("Пароль: ");
                }
                
                entry.url = readLine("URL сайта (необязательно): ");
                entry.note = readLine("Комментарий (необязательно): ");

                try {
                    pm.addEntry(entry);
                    std::cout << "Запись успешно добавлена!\n\n";
                } catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << "\n\n";
                }
                break;
            }
            case 2: { // Show All
                std::cout << "\n--- ВСЕ СОХРАНЕННЫЕ ПАРОЛИ ---\n";
                auto list = pm.getAllEntries();
                printEntriesTable(list, false);
                if (!list.empty()) {
                    std::cout << "Показать пароли в открытом виде? (y/n): ";
                    std::string ans;
                    std::getline(std::cin, ans);
                    if (ans == "y" || ans == "Y") {
                        printEntriesTable(list, true);
                    }
                }
                break;
            }
            case 3: { // Search
                std::cout << "\n--- ПОИСК ЗАПИСИ ---\n";
                std::string query = readLine("Введите название сервиса для поиска: ");
                auto results = pm.searchEntries(query);
                printEntriesTable(results, false);
                if (!results.empty()) {
                    std::cout << "Показать пароли для результатов поиска? (y/n): ";
                    std::string ans;
                    std::getline(std::cin, ans);
                    if (ans == "y" || ans == "Y") {
                        printEntriesTable(results, true);
                    }
                }
                break;
            }
            case 4: { // Edit
                std::cout << "\n--- РЕДАКТИРОВАНИЕ ЗАПИСИ ---\n";
                std::string service = readLine("Введите название сервиса для изменения: ");
                auto results = pm.searchEntries(service);
                if (results.empty()) {
                    std::cout << "Сервис не найден.\n\n";
                    break;
                }
                
                // Find exact match
                PasswordEntry current;
                bool found = false;
                for (const auto& r : results) {
                    if (r.serviceName == service) {
                        current = r;
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    std::cout << "Найдено совпадение, но не точное. Уточните название сервиса.\n";
                    printEntriesTable(results, false);
                    break;
                }

                std::cout << "Изменяем запись для: " << current.serviceName << "\n";
                std::cout << "(Оставьте строку пустой, если не хотите менять значение)\n";
                
                std::string user = readLine("Новый логин [" + current.username + "]: ");
                std::string pwd = readLine("Новый пароль [" + current.password + "]: ");
                std::string url = readLine("Новый URL [" + current.url + "]: ");
                std::string note = readLine("Новый комментарий [" + current.note + "]: ");

                PasswordEntry newDetails = current;
                if (!user.empty()) newDetails.username = user;
                if (!pwd.empty()) newDetails.password = pwd;
                if (!url.empty()) newDetails.url = url;
                if (!note.empty()) newDetails.note = note;

                try {
                    if (pm.editEntry(service, newDetails)) {
                        std::cout << "Запись успешно обновлена!\n\n";
                    } else {
                        std::cout << "Не удалось обновить запись.\n\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << "\n\n";
                }
                break;
            }
            case 5: { // Delete
                std::cout << "\n--- УДАЛЕНИЕ ЗАПИСИ ---\n";
                std::string service = readLine("Введите название сервиса для удаления: ");
                std::cout << "Вы уверены, что хотите удалить запись для '" << service << "'? (y/n): ";
                std::string confirm;
                std::getline(std::cin, confirm);
                if (confirm == "y" || confirm == "Y") {
                    if (pm.deleteEntry(service)) {
                        std::cout << "Запись успешно удалена!\n\n";
                    } else {
                        std::cout << "Запись с таким названием не найдена.\n\n";
                    }
                } else {
                    std::cout << "Удаление отменено.\n\n";
                }
                break;
            }
            case 6: { // Generate Password
                std::cout << "\n--- ГЕНЕРАЦИЯ ПАРОЛЯ ---\n";
                int len = 16;
                std::cout << "Длина пароля (8-64) [16]: ";
                std::string lenStr;
                std::getline(std::cin, lenStr);
                if (!lenStr.empty()) {
                    try {
                        len = std::stoi(lenStr);
                    } catch (...) {
                        std::cout << "Некорректное число, используется длина 16.\n";
                        len = 16;
                    }
                }

                std::string optUpper = readLine("Использовать заглавные буквы (A-Z)? (y/n) [y]: ");
                std::string optLower = readLine("Использовать строчные буквы (a-z)? (y/n) [y]: ");
                std::string optDigits = readLine("Использовать цифры (0-9)? (y/n) [y]: ");
                std::string optSpecial = readLine("Использовать спец. символы (!@#...)? (y/n) [y]: ");

                bool useUpper = optUpper.empty() || optUpper == "y" || optUpper == "Y";
                bool useLower = optLower.empty() || optLower == "y" || optLower == "Y";
                bool useDigits = optDigits.empty() || optDigits == "y" || optDigits == "Y";
                bool useSpecial = optSpecial.empty() || optSpecial == "y" || optSpecial == "Y";

                try {
                    std::string pwd = PasswordGenerator::generate(len, useUpper, useLower, useDigits, useSpecial);
                    std::cout << "\nСгенерированный пароль: " << pwd << "\n\n";
                } catch (const std::exception& e) {
                    std::cout << "Ошибка генерации: " << e.what() << "\n\n";
                }
                break;
            }
            case 7: { // Save and Exit
                try {
                    std::cout << "Сохранение базы паролей...\n";
                    pm.save();
                    std::cout << "Хранилище успешно сохранено в " << vaultPath << "\n";
                } catch (const std::exception& e) {
                    std::cout << "Ошибка при сохранении: " << e.what() << "\n";
                }
                pm.lock();
                std::cout << "До свидания!\n";
                running = false;
                break;
            }
            default: {
                std::cout << "Неверный пункт меню. Выберите 1-7.\n\n";
                break;
            }
        }
    }

    return 0;
}
