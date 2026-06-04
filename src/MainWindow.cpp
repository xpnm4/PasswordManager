#include "MainWindow.h"
#include "PasswordGenerator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QGuiApplication>
#include <QClipboard>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent), pm("vault.dat"), hasSelection(false) {
    
    // Set window title and size
    setWindowTitle("Менеджер паролей C++");
    resize(900, 600);

    // Initialize Stacked Widget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Setup pages
    setupLoginUI();
    setupDashboardUI();
    setupStyles();

    // Determine initial page state
    if (!pm.hasVault()) {
        loginTitleLabel->setText("Создание нового хранилища");
        loginButton->setText("Создать хранилище");
    } else {
        loginTitleLabel->setText("Вход в хранилище");
        loginButton->setText("Разблокировать");
    }

    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() {}

void MainWindow::setupLoginUI() {
    loginPage = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(loginPage);
    layout->setAlignment(Qt::AlignCenter);

    // Central login card
    QFrame* card = new QFrame(loginPage);
    card->setObjectName("loginCard");
    card->setFixedSize(400, 360);
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 25, 30, 25);
    cardLayout->setSpacing(12);

    loginTitleLabel = new QLabel("Вход в хранилище", card);
    loginTitleLabel->setObjectName("loginTitle");
    loginTitleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(loginTitleLabel);

    // Descriptive helper label
    QLabel* loginDescLabel = new QLabel(
        "Этот мастер-пароль защищает всю базу данных. Локальный сейф шифруется "
        "алгоритмом AES-256. Рекомендуется использовать надежный пароль.", card);
    loginDescLabel->setObjectName("loginDesc");
    loginDescLabel->setWordWrap(true);
    loginDescLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(loginDescLabel);

    masterPasswordInput = new QLineEdit(card);
    masterPasswordInput->setPlaceholderText("Мастер-пароль");
    masterPasswordInput->setEchoMode(QLineEdit::Password);
    masterPasswordInput->setObjectName("masterPasswordInput");
    masterPasswordInput->setToolTip("Введите мастер-пароль для шифрования/дешифрования сейфа");
    cardLayout->addWidget(masterPasswordInput);

    loginButton = new QPushButton("Войти", card);
    loginButton->setObjectName("loginButton");
    loginButton->setToolTip("Открыть хранилище паролей");
    cardLayout->addWidget(loginButton);

    // Warning label for password loss
    QLabel* warningLabel = new QLabel(
        "⚠️ Внимание: При потере мастер-пароля восстановить данные невозможно!", card);
    warningLabel->setObjectName("warningLabel");
    warningLabel->setWordWrap(true);
    warningLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(warningLabel);

    loginErrorLabel = new QLabel("", card);
    loginErrorLabel->setObjectName("errorLabel");
    loginErrorLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(loginErrorLabel);

    layout->addWidget(card);

    // Connect enter key and button click
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(masterPasswordInput, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);

    stackedWidget->addWidget(loginPage);
}

void MainWindow::setupDashboardUI() {
    dashboardPage = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(dashboardPage);
    pageLayout->setContentsMargins(15, 12, 15, 12);
    pageLayout->setSpacing(12);

    // --- TOP HEADER BAR ---
    QFrame* headerBar = new QFrame(dashboardPage);
    headerBar->setObjectName("headerBar");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(15, 8, 15, 8);

    QLabel* headerTitle = new QLabel("🛡️ Мой Защищенный Сейф Паролей", headerBar);
    headerTitle->setObjectName("headerTitle");
    headerLayout->addWidget(headerTitle);

    headerLayout->addStretch();

    QLabel* headerStatus = new QLabel("🔒 База данных: Зашифрована (AES-256)", headerBar);
    headerStatus->setObjectName("headerStatus");
    headerLayout->addWidget(headerStatus);

    pageLayout->addWidget(headerBar);

    // --- MAIN CONTENT SPLIT LAYOUT ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // --- LEFT COLUMN: SEARCH AND TABLE ---
    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setSpacing(10);

    // Search bar helper label
    QLabel* searchLabel = new QLabel("🔍 Поиск записей по названию сервиса:", dashboardPage);
    searchLabel->setObjectName("sectionTitle");
    leftCol->addWidget(searchLabel);

    // Search input
    searchInput = new QLineEdit(dashboardPage);
    searchInput->setPlaceholderText("Начните вводить название (например, GitHub)...");
    searchInput->setObjectName("searchInput");
    searchInput->setToolTip("Введите текст для мгновенной фильтрации списка паролей ниже");
    leftCol->addWidget(searchInput);

    // Table of entries
    entriesTable = new QTableWidget(dashboardPage);
    entriesTable->setColumnCount(3);
    entriesTable->setHorizontalHeaderLabels({"Сервис", "Логин / Email", "Адрес сайта (URL)"});
    entriesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    entriesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    entriesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    entriesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    entriesTable->setObjectName("entriesTable");
    entriesTable->setToolTip("Выберите запись кликом мыши, чтобы просмотреть её подробности справа");
    leftCol->addWidget(entriesTable);

    // Toolbar underneath the table
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    addBtn = new QPushButton("➕ Добавить новую", dashboardPage);
    addBtn->setObjectName("addBtn");
    addBtn->setToolTip("Создать новую запись с паролем");
    toolbarLayout->addWidget(addBtn);

    editBtn = new QPushButton("✏️ Изменить", dashboardPage);
    editBtn->setObjectName("editBtn");
    editBtn->setEnabled(false);
    editBtn->setToolTip("Редактировать выделенную запись");
    toolbarLayout->addWidget(editBtn);

    deleteBtn = new QPushButton("❌ Удалить", dashboardPage);
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setEnabled(false);
    deleteBtn->setToolTip("Удалить выделенную запись навсегда");
    toolbarLayout->addWidget(deleteBtn);

    leftCol->addLayout(toolbarLayout);
    contentLayout->addLayout(leftCol, 3); // 3/5 width

    // --- RIGHT COLUMN: DETAILS PANEL & TOOLS ---
    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setSpacing(15);

    // Details panel (Card view)
    detailsPanel = new QFrame(dashboardPage);
    detailsPanel->setObjectName("detailsPanel");
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsPanel);
    detailsLayout->setContentsMargins(20, 20, 20, 20);
    detailsLayout->setSpacing(12);

    QLabel* detailsHeader = new QLabel("📋 Детали учетной записи", detailsPanel);
    detailsHeader->setObjectName("detailsHeader");
    detailsLayout->addWidget(detailsHeader);

    // Form layout for details
    QFormLayout* form = new QFormLayout();
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    detailServiceLabel = new QLabel("-", detailsPanel);
    form->addRow("📌 Сервис:", detailServiceLabel);

    detailUsernameLabel = new QLabel("-", detailsPanel);
    form->addRow("👤 Логин / Email:", detailUsernameLabel);

    // Password row with show/copy controls
    QWidget* pwdContainer = new QWidget(detailsPanel);
    QHBoxLayout* pwdLayout = new QHBoxLayout(pwdContainer);
    pwdLayout->setContentsMargins(0, 0, 0, 0);
    pwdLayout->setSpacing(5);

    detailPasswordInput = new QLineEdit("-", pwdContainer);
    detailPasswordInput->setReadOnly(true);
    detailPasswordInput->setEchoMode(QLineEdit::Password);
    detailPasswordInput->setObjectName("detailPasswordInput");
    pwdLayout->addWidget(detailPasswordInput);

    togglePasswordButton = new QPushButton("Показать", pwdContainer);
    togglePasswordButton->setObjectName("miniBtn");
    togglePasswordButton->setToolTip("Показать/скрыть символы пароля");
    pwdLayout->addWidget(togglePasswordButton);

    copyPasswordButton = new QPushButton("Копировать", pwdContainer);
    copyPasswordButton->setObjectName("miniBtn");
    copyPasswordButton->setToolTip("Скопировать пароль в буфер обмена");
    pwdLayout->addWidget(copyPasswordButton);
    form->addRow("🔑 Пароль:", pwdContainer);

    detailUrlLabel = new QLabel("-", detailsPanel);
    form->addRow("🌐 Ссылка (URL):", detailUrlLabel);

    detailNoteLabel = new QLabel("-", detailsPanel);
    detailNoteLabel->setWordWrap(true);
    form->addRow("📝 Комментарий:", detailNoteLabel);

    detailCreatedLabel = new QLabel("-", detailsPanel);
    form->addRow("📅 Создана:", detailCreatedLabel);

    detailUpdatedLabel = new QLabel("-", detailsPanel);
    form->addRow("🔄 Изменена:", detailUpdatedLabel);

    detailsLayout->addLayout(form);
    detailsLayout->addStretch();
    rightCol->addWidget(detailsPanel, 4);

    // Tools Card
    QFrame* toolsPanel = new QFrame(dashboardPage);
    toolsPanel->setObjectName("toolsPanel");
    QHBoxLayout* toolsLayout = new QHBoxLayout(toolsPanel);
    toolsLayout->setContentsMargins(10, 10, 10, 10);
    toolsLayout->setSpacing(10);

    generatorBtn = new QPushButton("🎲 Генератор паролей", toolsPanel);
    generatorBtn->setObjectName("toolBtn");
    generatorBtn->setToolTip("Открыть мастер создания случайных надежных паролей");
    toolsLayout->addWidget(generatorBtn);

    lockBtn = new QPushButton("🔒 Заблокировать", toolsPanel);
    lockBtn->setObjectName("toolBtn");
    lockBtn->setToolTip("Сохранить изменения, заблокировать сейф и выйти");
    toolsLayout->addWidget(lockBtn);

    rightCol->addWidget(toolsPanel, 1);
    contentLayout->addLayout(rightCol, 2); // 2/5 width

    pageLayout->addLayout(contentLayout);

    // Connect dashboard events
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::handleSearch);
    connect(entriesTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::handleTableSelection);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::handleAddEntry);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::handleEditEntry);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::handleDeleteEntry);
    connect(togglePasswordButton, &QPushButton::clicked, this, &MainWindow::handleTogglePasswordVisibility);
    connect(copyPasswordButton, &QPushButton::clicked, this, &MainWindow::handleCopyPassword);
    connect(generatorBtn, &QPushButton::clicked, this, &MainWindow::handleOpenGenerator);
    connect(lockBtn, &QPushButton::clicked, this, &MainWindow::handleLock);

    stackedWidget->addWidget(dashboardPage);
}

void MainWindow::setupStyles() {
    QString qss = R"(
        QMainWindow {
            background-color: #121212;
        }
        
        /* Top Header Bar */
        #headerBar {
            background-color: #1a1a1a;
            border: 1px solid #2a2a2a;
            border-radius: 8px;
        }
        
        #headerTitle {
            font-size: 16px;
            font-weight: bold;
            color: #00adb5;
        }
        
        #headerStatus {
            font-size: 12px;
            color: #888888;
        }

        #sectionTitle {
            font-size: 13px;
            font-weight: bold;
            color: #aaaaaa;
        }

        /* Login Card */
        #loginCard {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f1f1f, stop:1 #151515);
            border: 2px solid #2d2d2d;
            border-radius: 12px;
        }
        
        #loginTitle {
            font-size: 22px;
            font-weight: bold;
            color: #00adb5;
        }
        
        #loginDesc {
            font-size: 12px;
            color: #aaaaaa;
            line-height: 1.4;
        }
        
        #warningLabel {
            font-size: 11px;
            color: #ff9800;
        }
        
        #errorLabel {
            color: #ff5252;
            font-size: 13px;
            font-weight: bold;
        }

        /* Forms inputs */
        QLineEdit {
            background-color: #242424;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            color: #ffffff;
            padding: 8px 12px;
            font-size: 13px;
        }
        
        QLineEdit:focus {
            border: 1px solid #00adb5;
            background-color: #2c2c2c;
        }

        /* Buttons */
        QPushButton {
            background-color: #0277bd;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #01579b;
        }
        
        QPushButton:pressed {
            background-color: #004d40;
        }
        
        QPushButton:disabled {
            background-color: #2b2b2b;
            color: #555555;
            border: 1px solid #222222;
        }
        
        #addBtn {
            background-color: #00796b;
        }
        #addBtn:hover {
            background-color: #004d40;
        }
        
        #editBtn {
            background-color: #ef6c00;
        }
        #editBtn:hover {
            background-color: #e65100;
        }

        #deleteBtn {
            background-color: #c62828;
        }
        #deleteBtn:hover {
            background-color: #b71c1c;
        }

        #miniBtn {
            font-size: 11px;
            padding: 5px 10px;
            background-color: #2a2a2a;
            color: #e0e0e0;
            border: 1px solid #3d3d3d;
        }

        #miniBtn:hover {
            background-color: #3d3d3d;
            border-color: #00adb5;
        }
        
        #toolBtn {
            background-color: #37474f;
        }
        #toolBtn:hover {
            background-color: #263238;
        }

        /* Panels */
        #detailsPanel, #toolsPanel {
            background-color: #181818;
            border: 1px solid #2a2a2a;
            border-radius: 8px;
        }
        
        #detailsHeader {
            font-size: 15px;
            font-weight: bold;
            color: #ffffff;
            border-bottom: 2px solid #00adb5;
            padding-bottom: 6px;
            margin-bottom: 10px;
        }
        
        QLabel {
            color: #d0d0d0;
            font-size: 13px;
        }
        
        /* Table Widget */
        QTableWidget {
            background-color: #181818;
            border: 1px solid #2a2a2a;
            gridline-color: #252525;
            color: #e0e0e0;
            border-radius: 8px;
        }
        
        QTableWidget::item {
            padding: 6px;
        }
        
        QTableWidget::item:selected {
            background-color: #0078d4;
            color: #ffffff;
        }
        
        QHeaderView::section {
            background-color: #2c2c2c;
            color: #ffffff;
            padding: 6px;
            border: 1px solid #2d2d2d;
            font-weight: bold;
        }
        
        QScrollBar:vertical {
            border: none;
            background-color: #1e1e1e;
            width: 10px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #3a3a3a;
            min-height: 20px;
            border-radius: 5px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #4f4f4f;
        }
    )";
    setStyleSheet(qss);
}

void MainWindow::handleLogin() {
    QString enteredPassword = masterPasswordInput->text();
    masterPasswordInput->clear();
    loginErrorLabel->clear();

    if (enteredPassword.isEmpty()) {
        loginErrorLabel->setText("Пароль не может быть пустым!");
        return;
    }

    try {
        if (!pm.hasVault()) {
            pm.createVault(enteredPassword.toStdString());
            QMessageBox::information(this, "Успех", "Хранилище создано успешно!");
            stackedWidget->setCurrentWidget(dashboardPage);
            refreshEntriesList();
        } else {
            if (pm.unlock(enteredPassword.toStdString())) {
                stackedWidget->setCurrentWidget(dashboardPage);
                refreshEntriesList();
            } else {
                loginErrorLabel->setText("Неверный мастер-пароль!");
            }
        }
    } catch (const std::exception& e) {
        loginErrorLabel->setText(QString("Ошибка: ") + e.what());
    }
}

void MainWindow::refreshEntriesList(const QString& query) {
    entriesTable->clearContents();
    clearEntryDetails();

    std::vector<PasswordEntry> list;
    if (query.isEmpty()) {
        list = pm.getAllEntries();
    } else {
        list = pm.searchEntries(query.toStdString());
    }

    entriesTable->setRowCount(static_cast<int>(list.size()));
    for (size_t i = 0; i < list.size(); ++i) {
        // We store the full entry in row properties or recreate it.
        // Let's create items
        auto* itemService = new QTableWidgetItem(QString::fromStdString(list[i].serviceName));
        auto* itemUser = new QTableWidgetItem(QString::fromStdString(list[i].username));
        auto* itemUrl = new QTableWidgetItem(QString::fromStdString(list[i].url));

        entriesTable->setItem(static_cast<int>(i), 0, itemService);
        entriesTable->setItem(static_cast<int>(i), 1, itemUser);
        entriesTable->setItem(static_cast<int>(i), 2, itemUrl);
    }
    
    // Disable edit/delete buttons since selection is cleared
    editBtn->setEnabled(false);
    deleteBtn->setEnabled(false);
    hasSelection = false;
}

void MainWindow::handleTableSelection() {
    int row = entriesTable->currentRow();
    if (row < 0 || row >= entriesTable->rowCount()) {
        clearEntryDetails();
        return;
    }

    QString serviceName = entriesTable->item(row, 0)->text();
    
    // Find entry
    auto list = pm.getAllEntries();
    for (const auto& e : list) {
        if (QString::fromStdString(e.serviceName) == serviceName) {
            selectedEntry = e;
            hasSelection = true;
            showEntryDetails(e);
            editBtn->setEnabled(true);
            deleteBtn->setEnabled(true);
            return;
        }
    }
}

void MainWindow::showEntryDetails(const PasswordEntry& entry) {
    detailServiceLabel->setText(QString::fromStdString(entry.serviceName));
    detailUsernameLabel->setText(QString::fromStdString(entry.username));
    
    detailPasswordInput->setText(QString::fromStdString(entry.password));
    detailPasswordInput->setEchoMode(QLineEdit::Password);
    togglePasswordButton->setText("Показать");

    detailUrlLabel->setText(entry.url.empty() ? "-" : QString::fromStdString(entry.url));
    detailNoteLabel->setText(entry.note.empty() ? "-" : QString::fromStdString(entry.note));
    detailCreatedLabel->setText(QString::fromStdString(entry.createdAt));
    detailUpdatedLabel->setText(QString::fromStdString(entry.updatedAt));
}

void MainWindow::clearEntryDetails() {
    detailServiceLabel->setText("Выберите запись из списка...");
    detailUsernameLabel->setText("Выберите запись из списка...");
    detailPasswordInput->setText("");
    detailPasswordInput->setEchoMode(QLineEdit::Password);
    togglePasswordButton->setText("Показать");
    detailUrlLabel->setText("Выберите запись из списка...");
    detailNoteLabel->setText("Выберите запись из списка...");
    detailCreatedLabel->setText("-");
    detailUpdatedLabel->setText("-");
    hasSelection = false;
}

void MainWindow::handleTogglePasswordVisibility() {
    if (!hasSelection) return;
    if (detailPasswordInput->echoMode() == QLineEdit::Password) {
        detailPasswordInput->setEchoMode(QLineEdit::Normal);
        togglePasswordButton->setText("Скрыть");
    } else {
        detailPasswordInput->setEchoMode(QLineEdit::Password);
        togglePasswordButton->setText("Показать");
    }
}

void MainWindow::handleCopyPassword() {
    if (!hasSelection) return;
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::fromStdString(selectedEntry.password));
    // Toast notification or tray message would be neat, but standard status bar is fine:
    statusBar()->showMessage("Пароль скопирован в буфер обмена!", 2000);
}

void MainWindow::handleSearch(const QString& text) {
    refreshEntriesList(text);
}

void MainWindow::handleAddEntry() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавление новой записи");
    dialog.setMinimumWidth(350);

    QFormLayout form(&dialog);
    QLineEdit serviceEdit(&dialog);
    QLineEdit userEdit(&dialog);
    
    // Password line edit with generate button next to it
    QWidget* pwdWidget = new QWidget(&dialog);
    QHBoxLayout* pwdLayout = new QHBoxLayout(pwdWidget);
    pwdLayout->setContentsMargins(0, 0, 0, 0);
    QLineEdit pwdEdit(pwdWidget);
    QPushButton genBtn("Сген.", pwdWidget);
    pwdLayout->addWidget(&pwdEdit);
    pwdLayout->addWidget(&genBtn);

    QLineEdit urlEdit(&dialog);
    QTextEdit noteEdit(&dialog);
    noteEdit.setMaximumHeight(80);

    form.addRow("Сервис *:", &serviceEdit);
    form.addRow("Логин:", &userEdit);
    form.addRow("Пароль:", pwdWidget);
    form.addRow("URL:", &urlEdit);
    form.addRow("Заметка:", &noteEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    connect(&genBtn, &QPushButton::clicked, [&]() {
        try {
            std::string randomPass = PasswordGenerator::generate(16, true, true, true, true);
            pwdEdit.setText(QString::fromStdString(randomPass));
        } catch (...) {}
    });

    if (dialog.exec() == QDialog::Accepted) {
        if (serviceEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название сервиса обязательно!");
            return;
        }

        PasswordEntry entry;
        entry.serviceName = serviceEdit.text().trimmed().toStdString();
        entry.username = userEdit.text().trimmed().toStdString();
        entry.password = pwdEdit.text().toStdString();
        entry.url = urlEdit.text().trimmed().toStdString();
        entry.note = noteEdit.toPlainText().trimmed().toStdString();

        try {
            pm.addEntry(entry);
            pm.save();
            refreshEntriesList(searchInput->text());
            statusBar()->showMessage("Запись успешно добавлена!", 3000);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Ошибка", e.what());
        }
    }
}

void MainWindow::handleEditEntry() {
    if (!hasSelection) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Редактирование записи: " + QString::fromStdString(selectedEntry.serviceName));
    dialog.setMinimumWidth(350);

    QFormLayout form(&dialog);
    QLineEdit userEdit(&dialog);
    userEdit.setText(QString::fromStdString(selectedEntry.username));

    // Password line edit with generate button
    QWidget* pwdWidget = new QWidget(&dialog);
    QHBoxLayout* pwdLayout = new QHBoxLayout(pwdWidget);
    pwdLayout->setContentsMargins(0, 0, 0, 0);
    QLineEdit pwdEdit(pwdWidget);
    pwdEdit.setText(QString::fromStdString(selectedEntry.password));
    QPushButton genBtn("Сген.", pwdWidget);
    pwdLayout->addWidget(&pwdEdit);
    pwdLayout->addWidget(&genBtn);

    QLineEdit urlEdit(&dialog);
    urlEdit.setText(QString::fromStdString(selectedEntry.url));

    QTextEdit noteEdit(&dialog);
    noteEdit.setText(QString::fromStdString(selectedEntry.note));
    noteEdit.setMaximumHeight(80);

    form.addRow("Логин:", &userEdit);
    form.addRow("Пароль:", pwdWidget);
    form.addRow("URL:", &urlEdit);
    form.addRow("Заметка:", &noteEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    connect(&genBtn, &QPushButton::clicked, [&]() {
        try {
            std::string randomPass = PasswordGenerator::generate(16, true, true, true, true);
            pwdEdit.setText(QString::fromStdString(randomPass));
        } catch (...) {}
    });

    if (dialog.exec() == QDialog::Accepted) {
        PasswordEntry newDetails = selectedEntry;
        newDetails.username = userEdit.text().trimmed().toStdString();
        newDetails.password = pwdEdit.text().toStdString();
        newDetails.url = urlEdit.text().trimmed().toStdString();
        newDetails.note = noteEdit.toPlainText().trimmed().toStdString();

        try {
            if (pm.editEntry(selectedEntry.serviceName, newDetails)) {
                pm.save();
                refreshEntriesList(searchInput->text());
                statusBar()->showMessage("Запись успешно обновлена!", 3000);
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Ошибка", e.what());
        }
    }
}

void MainWindow::handleDeleteEntry() {
    if (!hasSelection) return;

    auto result = QMessageBox::question(this, "Удаление", 
        QString("Вы уверены, что хотите удалить запись для '%1'?")
            .arg(QString::fromStdString(selectedEntry.serviceName)),
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        try {
            if (pm.deleteEntry(selectedEntry.serviceName)) {
                pm.save();
                refreshEntriesList(searchInput->text());
                statusBar()->showMessage("Запись успешно удалена!", 3000);
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Ошибка", e.what());
        }
    }
}

void MainWindow::handleOpenGenerator() {
    QDialog dialog(this);
    dialog.setWindowTitle("Генератор безопасных паролей");
    dialog.setMinimumWidth(350);

    QVBoxLayout layout(&dialog);

    // Form layout for settings
    QFormLayout form;
    QSpinBox lenSpin(&dialog);
    lenSpin.setRange(8, 64);
    lenSpin.setValue(16);
    form.addRow("Длина пароля:", &lenSpin);

    QCheckBox upperChk("Использовать заглавные буквы (A-Z)", &dialog);
    upperChk.setChecked(true);
    form.addRow(&upperChk);

    QCheckBox lowerChk("Использовать строчные буквы (a-z)", &dialog);
    lowerChk.setChecked(true);
    form.addRow(&lowerChk);

    QCheckBox digitChk("Использовать цифры (0-9)", &dialog);
    digitChk.setChecked(true);
    form.addRow(&digitChk);

    QCheckBox specChk("Использовать спец. символы (!@#...)", &dialog);
    specChk.setChecked(true);
    form.addRow(&specChk);

    layout.addLayout(&form);

    // Text box with result
    QLineEdit resultField(&dialog);
    resultField.setReadOnly(true);
    resultField.setAlignment(Qt::AlignCenter);
    QFont f = resultField.font();
    f.setPointSize(12);
    f.setBold(true);
    resultField.setFont(f);
    layout.addWidget(&resultField);

    QHBoxLayout btnLayout;
    QPushButton genBtn("Сгенерировать", &dialog);
    QPushButton copyBtn("Копировать", &dialog);
    btnLayout.addWidget(&genBtn);
    btnLayout.addWidget(&copyBtn);
    layout.addLayout(&btnLayout);

    QDialogButtonBox buttonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    auto generateFn = [&]() {
        try {
            std::string pwd = PasswordGenerator::generate(
                lenSpin.value(),
                upperChk.isChecked(),
                lowerChk.isChecked(),
                digitChk.isChecked(),
                specChk.isChecked()
            );
            resultField.setText(QString::fromStdString(pwd));
        } catch (const std::exception& e) {
            QMessageBox::warning(&dialog, "Ошибка", e.what());
        }
    };

    connect(&genBtn, &QPushButton::clicked, generateFn);
    
    connect(&copyBtn, &QPushButton::clicked, [&]() {
        if (!resultField.text().isEmpty()) {
            QGuiApplication::clipboard()->setText(resultField.text());
            statusBar()->showMessage("Сгенерированный пароль скопирован!", 2000);
        }
    });

    // Run first generation automatically
    generateFn();

    dialog.exec();
}

void MainWindow::handleLock() {
    pm.lock();
    clearEntryDetails();
    masterPasswordInput->clear();
    loginErrorLabel->clear();

    if (!pm.hasVault()) {
        loginTitleLabel->setText("Создание нового хранилища");
        loginButton->setText("Создать хранилище");
    } else {
        loginTitleLabel->setText("Вход в хранилище");
        loginButton->setText("Разблокировать");
    }

    stackedWidget->setCurrentWidget(loginPage);
    statusBar()->showMessage("База данных заблокирована.", 3000);
}
