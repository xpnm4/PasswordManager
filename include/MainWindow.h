#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <QGroupBox>
#include "PasswordManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    PasswordManager pm;

    // Stacked widget to switch between Login Screen and Dashboard
    QStackedWidget* stackedWidget;

    // --- LOGIN WIDGETS ---
    QWidget* loginPage;
    QLineEdit* masterPasswordInput;
    QPushButton* loginButton;
    QLabel* loginErrorLabel;
    QLabel* loginTitleLabel;

    // --- DASHBOARD WIDGETS ---
    QWidget* dashboardPage;
    QLineEdit* searchInput;
    QTableWidget* entriesTable;
    
    // Details panel widgets
    QLabel* detailServiceLabel;
    QLabel* detailUsernameLabel;
    QLineEdit* detailPasswordInput; // QLineEdit to easily select/copy/mask
    QPushButton* togglePasswordButton;
    QPushButton* copyPasswordButton;
    QLabel* detailUrlLabel;
    QLabel* detailNoteLabel;
    QLabel* detailCreatedLabel;
    QLabel* detailUpdatedLabel;
    QWidget* detailsPanel;

    // Dashboard Buttons
    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* generatorBtn;
    QPushButton* lockBtn;

    // Selected entry tracking
    bool hasSelection;
    PasswordEntry selectedEntry;

    // Setup UI Helper functions
    void setupLoginUI();
    void setupDashboardUI();
    void setupStyles();
    void refreshEntriesList(const QString& query = "");
    void showEntryDetails(const PasswordEntry& entry);
    void clearEntryDetails();

private slots:
    // Authentication Slots
    void handleLogin();

    // Entries Slots
    void handleAddEntry();
    void handleEditEntry();
    void handleDeleteEntry();
    void handleSearch(const QString& text);
    void handleTableSelection();

    // Detail Panel Slots
    void handleTogglePasswordVisibility();
    void handleCopyPassword();

    // Tools Slots
    void handleOpenGenerator();
    void handleLock();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
};

#endif // MAINWINDOW_H
