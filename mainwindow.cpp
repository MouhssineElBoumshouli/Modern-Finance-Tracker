#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QMenu>
#include <QFileDialog>
#include <QTextStream>
#include <QDialog>
#include <QGridLayout>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QShortcut>
#include <QMenuBar>
#include <QGroupBox>
#include <QScrollArea>
#include <QScreen>
#include <cmath>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QPieSlice>

QT_USE_NAMESPACE
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentBalance(0.0)
    , totalIncome(0.0)
    , totalExpenses(0.0)
{
    // Initialize database
    if (!dbManager.initialize()) {
        QMessageBox::critical(this, "Error", "Failed to initialize database!");
    }

    // Setup UI
    setupUI();

    // Load data
    loadTransactionsFromDatabase();
}
MainWindow::~MainWindow()
{    cleanupCharts();

}

void MainWindow::loadTransactionsFromDatabase()
{
    // Clear existing data
    transactions.clear();
    totalIncome = 0.0;
    totalExpenses = 0.0;
    currentBalance = 0.0;

    // Load transactions
    transactions = dbManager.getAllTransactions();

    // Recalculate totals
    for (const Transaction& trans : transactions) {
        if (trans.type() == Transaction::Income) {
            totalIncome += trans.amount();
            currentBalance += trans.amount();
        } else {
            totalExpenses += std::abs(trans.amount());
            currentBalance -= std::abs(trans.amount());
        }
    }

    // Update UI
    updateBalance();
    updateTransactionTable();
    updateAnalytics();
}
void MainWindow::setupUI()
{
    // Set window properties
    setWindowTitle("Modern Finance Tracker");
    resize(1000, 600);

    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Setup navigation
    setupNavigation();

    // Create page stack
    pageStack = new QStackedWidget;
    mainLayout->addWidget(pageStack);

    // Create pages
    dashboardPage = new QWidget;
    transactionsPage = new QWidget;
    analyticsPage = new QWidget;

    // Add pages to stack
    pageStack->addWidget(dashboardPage);
    pageStack->addWidget(transactionsPage);
    pageStack->addWidget(analyticsPage);

    // Setup pages
    setupDashboard();
    setupTransactionsPage();
    setupAnalyticsPage();

    // Show dashboard by default
    showDashboard();

    // Set initial theme
    setTheme(false);

    // Setup keyboard shortcuts
    newTransactionShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    deleteTransactionShortcut = new QShortcut(QKeySequence("Delete"), this);
    searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    refreshShortcut = new QShortcut(QKeySequence("F5"), this);

    connect(newTransactionShortcut, &QShortcut::activated,
            this, &MainWindow::newTransactionShortcutTriggered);
    connect(deleteTransactionShortcut, &QShortcut::activated,
            this, &MainWindow::deleteSelectedTransaction);
    connect(searchShortcut, &QShortcut::activated,
            this, &MainWindow::focusSearchBox);
    connect(refreshShortcut, &QShortcut::activated,
            this, &MainWindow::updateTransactionTable);

    // Add Help menu
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *shortcutsAction = helpMenu->addAction("Keyboard Shortcuts");
    connect(shortcutsAction, &QAction::triggered, this, &MainWindow::showShortcutsDialog);
}
void MainWindow::updateAnalytics()
{
    // Simply recreate the analytics page
    delete analyticsPage->layout();
    setupAnalyticsPage();
}

void MainWindow::showShortcutsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Keyboard Shortcuts");
    dialog.setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *title = new QLabel("Available Keyboard Shortcuts", &dialog);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    title->setFont(titleFont);
    layout->addWidget(title);

    // Create a grid for shortcuts
    QGridLayout *grid = new QGridLayout;
    int row = 0;

    auto addShortcut = [&](const QString& key, const QString& description) {
        QLabel *keyLabel = new QLabel(key, &dialog);
        keyLabel->setStyleSheet("font-weight: bold; padding: 5px; background-color: #f0f0f0; border-radius: 3px;");

        grid->addWidget(keyLabel, row, 0);
        grid->addWidget(new QLabel(description, &dialog), row, 1);
        row++;
    };

    addShortcut("Ctrl + N", "Add new transaction");
    addShortcut("Delete", "Delete selected transaction");
    addShortcut("Ctrl + F", "Focus search box");
    addShortcut("F5", "Refresh transaction list");

    layout->addLayout(grid);
    layout->addStretch();

    QPushButton *closeButton = new QPushButton("Close", &dialog);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeButton);

    dialog.exec();
}
void MainWindow::newTransactionShortcutTriggered()
{
    showTransactions();
    amountEdit->setFocus();
}

void MainWindow::focusSearchBox()
{
    searchEdit->setFocus();
}
void MainWindow::setupTransactionsPage()
{
    QVBoxLayout *layout = new QVBoxLayout(transactionsPage);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    // Create transaction form
    setupTransactionForm();

    // Create transaction table
    setupTransactionTable();

    // Add export buttons
    QHBoxLayout *exportLayout = new QHBoxLayout;
    QPushButton *exportCsvBtn = new QPushButton("Export to CSV");
    QPushButton *exportPdfBtn = new QPushButton("Export to PDF");
    QPushButton *exportExcelBtn = new QPushButton("Export to Excel");

    exportLayout->addStretch();
    exportLayout->addWidget(exportCsvBtn);
    exportLayout->addWidget(exportPdfBtn);
    exportLayout->addWidget(exportExcelBtn);

    connect(exportCsvBtn, &QPushButton::clicked, this, &MainWindow::exportToCSV);
    connect(exportPdfBtn, &QPushButton::clicked, this, &MainWindow::exportToPDF);
    connect(exportExcelBtn, &QPushButton::clicked, this, &MainWindow::exportToExcel);

    layout->addLayout(exportLayout);
}

// Add the rest of your existing methods here (setupTransactionForm, setupTransactionTable, etc.)
// but remove all chart-related code for now.

void MainWindow::exportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export Transactions", "", "CSV Files (*.csv)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Write header
    out << "Date,Type,Amount,Description,Category\n";

    // Write transactions
    for (const Transaction& trans : transactions) {
        out << trans.datetime().toString("yyyy-MM-dd hh:mm") << ","
            << (trans.type() == Transaction::Income ? "Income" : "Expense") << ","
            << QString::number(std::abs(trans.amount()), 'f', 2) << ","
            << "\"" << trans.description().replace("\"", "\"\"") << "\"" << ","
            << trans.category() << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Transactions exported successfully!");
}

void MainWindow::exportToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export Transactions", "", "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument doc;
    QString html = "<h1>Finance Tracker - Transaction Report</h1>";
    html += "<p>Generated on: " + QDateTime::currentDateTime().toString() + "</p>";

    // Add summary
    html += "<h2>Summary</h2>";
    html += "<p>Total Income: $" + QString::number(totalIncome, 'f', 2) + "<br>";
    html += "Total Expenses: $" + QString::number(totalExpenses, 'f', 2) + "<br>";
    html += "Current Balance: $" + QString::number(currentBalance, 'f', 2) + "</p>";

    // Add transactions table
    html += "<h2>Transactions</h2>";
    html += "<table border='1' cellspacing='0' cellpadding='3' width='100%'>";
    html += "<tr bgcolor='#f0f0f0'><th>Date</th><th>Type</th><th>Amount</th><th>Description</th><th>Category</th></tr>";

    for (const Transaction& trans : transactions) {
        html += "<tr>";
        html += "<td>" + trans.datetime().toString("yyyy-MM-dd hh:mm") + "</td>";
        html += "<td>" + QString(trans.type() == Transaction::Income ? "Income" : "Expense") + "</td>";
        html += "<td align='right'>$" + QString::number(std::abs(trans.amount()), 'f', 2) + "</td>";
        html += "<td>" + trans.description() + "</td>";
        html += "<td>" + trans.category() + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Success", "Report exported to PDF successfully!");
}
void MainWindow::exportToExcel()
{
    // We'll use CSV since Excel can open it
    exportToCSV();
}
// Navigation methods
void MainWindow::setupNavigation()
{
    QFrame *navBar = new QFrame;
    navBar->setMaximumHeight(60);

    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setSpacing(10);
    navLayout->setContentsMargins(20, 10, 20, 10);

    // Create navigation buttons
    dashboardButton = new QPushButton("🏠 Dashboard");
    transactionsButton = new QPushButton("💰 Transactions");
    analyticsButton = new QPushButton("📊 Analytics");
    themeButton = new QPushButton("🌙");

    // Add buttons to navigation
    navLayout->addWidget(dashboardButton);
    navLayout->addWidget(transactionsButton);
    navLayout->addWidget(analyticsButton);
    navLayout->addStretch();
    navLayout->addWidget(themeButton);

    // Connect buttons
    connect(dashboardButton, &QPushButton::clicked, this, &MainWindow::showDashboard);
    connect(transactionsButton, &QPushButton::clicked, this, &MainWindow::showTransactions);
    connect(analyticsButton, &QPushButton::clicked, this, &MainWindow::showAnalytics);
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::toggleTheme);

    // Add navigation to main window
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    mainLayout->addWidget(navBar);
}

void MainWindow::setupDashboard()
{
    QVBoxLayout *layout = new QVBoxLayout(dashboardPage);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    // Balance section
    QGroupBox *balanceGroup = new QGroupBox("Financial Overview");
    QVBoxLayout *balanceLayout = new QVBoxLayout(balanceGroup);

    // Main balance
    balanceLabel = new QLabel("$0.00");
    balanceLabel->setAlignment(Qt::AlignCenter);
    QFont balanceFont = balanceLabel->font();
    balanceFont.setPointSize(24);
    balanceFont.setBold(true);
    balanceLabel->setFont(balanceFont);

    // Income and Expense labels
    QHBoxLayout *statsLayout = new QHBoxLayout;

    incomeLabel = new QLabel("Income: $0.00");
    incomeLabel->setStyleSheet("color: green;");

    expenseLabel = new QLabel("Expenses: $0.00");
    expenseLabel->setStyleSheet("color: red;");

    statsLayout->addWidget(incomeLabel);
    statsLayout->addWidget(expenseLabel);

    balanceLayout->addWidget(balanceLabel);
    balanceLayout->addLayout(statsLayout);

    layout->addWidget(balanceGroup);
    layout->addStretch();
}

void MainWindow::setupTransactionForm()
{
    QGroupBox *formGroup = new QGroupBox("Add New Transaction");
    QVBoxLayout *formLayout = new QVBoxLayout(formGroup);

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(10);

    // Type selector
    grid->addWidget(new QLabel("Type:"), 0, 0);
    typeCombo = new QComboBox;
    typeCombo->addItems({"Income", "Expense"});
    grid->addWidget(typeCombo, 0, 1);

    // Amount input
    grid->addWidget(new QLabel("Amount:"), 0, 2);
    amountEdit = new QLineEdit;
    amountEdit->setPlaceholderText("Enter amount");
    grid->addWidget(amountEdit, 0, 3);

    // Description input
    grid->addWidget(new QLabel("Description:"), 1, 0);
    descriptionEdit = new QLineEdit;
    descriptionEdit->setPlaceholderText("Enter description");
    grid->addWidget(descriptionEdit, 1, 1, 1, 3);

    // Category selector
    grid->addWidget(new QLabel("Category:"), 2, 0);
    categoryCombo = new QComboBox;
    categoryCombo->addItems({"Salary", "Food", "Transport", "Entertainment", "Bills", "Shopping", "Other"});
    grid->addWidget(categoryCombo, 2, 1);

    // Date and time picker
    grid->addWidget(new QLabel("Date:"), 2, 2);
    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    grid->addWidget(dateTimeEdit, 2, 3);

    formLayout->addLayout(grid);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("Add Transaction");
    clearButton = new QPushButton("Clear Form");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(clearButton);
    formLayout->addLayout(buttonLayout);

    // Connect buttons
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addNewTransaction);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearTransactionForm);

    // Add form to transactions page
    QVBoxLayout *pageLayout = qobject_cast<QVBoxLayout*>(transactionsPage->layout());
    pageLayout->addWidget(formGroup);
}

void MainWindow::setupTransactionTable()
{
    transactionTable = new QTableWidget;
    transactionTable->setColumnCount(5);
    transactionTable->setHorizontalHeaderLabels({"Type", "Amount", "Description", "Category", "Date/Time"});

    // Style the table
    transactionTable->setAlternatingRowColors(true);
    transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionTable->verticalHeader()->setVisible(false);

    // Enable context menu
    transactionTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(transactionTable, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::handleTransactionTableContextMenu);

    // Add table to transactions page
    QVBoxLayout *pageLayout = qobject_cast<QVBoxLayout*>(transactionsPage->layout());
    pageLayout->addWidget(transactionTable);
}

// Navigation slots
void MainWindow::showDashboard()
{
    pageStack->setCurrentWidget(dashboardPage);
    dashboardButton->setChecked(true);
    transactionsButton->setChecked(false);
    analyticsButton->setChecked(false);
}

void MainWindow::showTransactions()
{
    pageStack->setCurrentWidget(transactionsPage);
    dashboardButton->setChecked(false);
    transactionsButton->setChecked(true);
    analyticsButton->setChecked(false);
}
void MainWindow::showAnalytics()
{
    pageStack->setCurrentWidget(analyticsPage);
    dashboardButton->setChecked(false);
    transactionsButton->setChecked(false);
    analyticsButton->setChecked(true);
}

void MainWindow::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    setTheme(isDarkTheme);
    themeButton->setText(isDarkTheme ? "☀️" : "🌙");
}
void MainWindow::setTheme(bool darkTheme)
{
    QString baseStyle;
    if (darkTheme) {
        baseStyle = R"(
            QMainWindow, QWidget {
                background-color: #1e1e1e;
                color: #ffffff;
            }
            QPushButton {
                background-color: #333333;
                color: #ffffff;
                border: none;
                padding: 8px 16px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #404040;
            }
            QPushButton:checked {
                background-color: #0078d4;
            }
            QFrame {
                background-color: #252525;
                border-radius: 10px;
            }
            QGroupBox {
                background-color: #252525;
                border-radius: 10px;
                padding: 15px;
            }
            QTableWidget {
                background-color: #252525;
                alternate-background-color: #2d2d2d;
                border: none;
                color: #ffffff;
            }
            QHeaderView::section {
                background-color: #333333;
                color: #ffffff;
                padding: 5px;
                border: none;
            }
            QChartView {
                background-color: transparent;
            }
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollBar:vertical {
                border: none;
                background-color: #252525;
                width: 10px;
                margin: 0px;
            }
            QScrollBar::handle:vertical {
                background-color: #404040;
                min-height: 30px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #505050;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                background: none;
            }
        )";
    } else {
        baseStyle = R"(
            QMainWindow, QWidget {
                background-color: #ffffff;
                color: #000000;
            }
            QPushButton {
                background-color: #f0f0f0;
                color: #000000;
                border: none;
                padding: 8px 16px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #e0e0e0;
            }
            QPushButton:checked {
                background-color: #0078d4;
                color: #ffffff;
            }
            QFrame {
                background-color: #f8f8f8;
                border-radius: 10px;
            }
            QGroupBox {
                background-color: #f8f8f8;
                border-radius: 10px;
                padding: 15px;
            }
            QTableWidget {
                background-color: #ffffff;
                alternate-background-color: #f5f5f5;
                border: none;
                color: #000000;
            }
            QHeaderView::section {
                background-color: #f0f0f0;
                padding: 5px;
                border: none;
            }
            QChartView {
                background-color: transparent;
            }
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollBar:vertical {
                border: none;
                background-color: #f0f0f0;
                width: 10px;
                margin: 0px;
            }
            QScrollBar::handle:vertical {
                background-color: #c0c0c0;
                min-height: 30px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #a0a0a0;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                background: none;
            }
        )";
    }
    setStyleSheet(baseStyle);

    // Update charts theme
    if (expenseChartView && expenseChartView->chart()) {
        expenseChartView->chart()->setTheme(darkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);
        expenseChartView->chart()->setBackgroundVisible(false);
    }
    if (monthlyComparisonChart && monthlyComparisonChart->chart()) {
        monthlyComparisonChart->chart()->setTheme(darkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);
        monthlyComparisonChart->chart()->setBackgroundVisible(false);
    }
    if (balanceTrendChart && balanceTrendChart->chart()) {
        balanceTrendChart->chart()->setTheme(darkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);
        balanceTrendChart->chart()->setBackgroundVisible(false);
    }
}
void MainWindow::addNewTransaction()
{
    bool ok;
    double amount = amountEdit->text().toDouble(&ok);

    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid amount.");
        return;
    }

    Transaction::Type type = typeCombo->currentText() == "Income" ?
                                 Transaction::Income : Transaction::Expense;

    if (type == Transaction::Expense) {
        amount = -amount;  // Make expenses negative
    }

    Transaction transaction(type, amount, descriptionEdit->text(),
                            categoryCombo->currentText(), dateTimeEdit->dateTime());

    if (!dbManager.addTransaction(transaction)) {
        QMessageBox::critical(this, "Error", "Failed to save transaction to database!");
        return;
    }

    // Store transaction in vector
    transactions.append(transaction);

    // Add to table
    int row = transactionTable->rowCount();
    transactionTable->insertRow(row);

    QTableWidgetItem *typeItem = new QTableWidgetItem(type == Transaction::Income ? "Income" : "Expense");
    typeItem->setForeground(type == Transaction::Income ? Qt::darkGreen : Qt::red);

    transactionTable->setItem(row, 0, typeItem);
    transactionTable->setItem(row, 1, new QTableWidgetItem(QString::number(std::abs(amount), 'f', 2)));
    transactionTable->setItem(row, 2, new QTableWidgetItem(descriptionEdit->text()));
    transactionTable->setItem(row, 3, new QTableWidgetItem(categoryCombo->currentText()));
    transactionTable->setItem(row, 4, new QTableWidgetItem(dateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm")));

    // Update totals
    if (type == Transaction::Income) {
        totalIncome += std::abs(amount);
        currentBalance += amount;
    } else {
        totalExpenses += std::abs(amount);
        currentBalance += amount;  // amount is already negative
    }

    // Update UI
    updateBalance();
    delete analyticsPage->layout();
    setupAnalyticsPage();  // Refresh analytics

    // Clear form
    clearTransactionForm();

    QMessageBox::information(this, "Success", "Transaction added successfully!");
}

void MainWindow::setupAnalyticsPage()
{
    // First, clear everything from the analytics page
    if (analyticsPage->layout()) {
        QLayoutItem* item;
        while ((item = analyticsPage->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete analyticsPage->layout();
    }

    // Create a single main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(analyticsPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create single scroll area
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Create the content widget that will hold everything
    QWidget *scrollContent = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    // Overview Section
    QGroupBox *overviewGroup = new QGroupBox("Financial Overview");
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewGroup);

    QLabel *totalIncomeLabel = new QLabel(QString("Total Income: $%1").arg(totalIncome, 0, 'f', 2));
    QLabel *totalExpensesLabel = new QLabel(QString("Total Expenses: $%1").arg(totalExpenses, 0, 'f', 2));
    QLabel *netBalanceLabel = new QLabel(QString("Net Balance: $%1").arg(currentBalance, 0, 'f', 2));

    totalIncomeLabel->setStyleSheet("color: #2ecc71; font-size: 16px; padding: 10px;");
    totalExpensesLabel->setStyleSheet("color: #e74c3c; font-size: 16px; padding: 10px;");
    netBalanceLabel->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");

    overviewLayout->addWidget(totalIncomeLabel);
    overviewLayout->addWidget(totalExpensesLabel);
    overviewLayout->addWidget(netBalanceLabel);

    // Expense Pie Chart Section
    QGroupBox *pieChartGroup = new QGroupBox("Expenses by Category");
    QVBoxLayout *pieChartLayout = new QVBoxLayout(pieChartGroup);

    // Create pie chart
    QPieSeries *pieSeries = new QPieSeries();

    // Calculate category totals for pie chart
    QMap<QString, double> categoryTotals;
    for (const Transaction& trans : transactions) {
        if (trans.type() == Transaction::Expense) {
            categoryTotals[trans.category()] += std::abs(trans.amount());
        }
    }

    // Add slices to pie chart with consistent colors
    QStringList colors = {
        "#2ecc71", "#e74c3c", "#3498db", "#f1c40f",
        "#9b59b6", "#1abc9c", "#e67e22", "#34495e"
    };
    int colorIndex = 0;

    for (auto it = categoryTotals.begin(); it != categoryTotals.end(); ++it) {
        double percentage = (totalExpenses > 0) ? (it.value() / totalExpenses * 100) : 0;
        QPieSlice *slice = pieSeries->append(it.key(), it.value());
        slice->setLabel(QString("%1\n$%2 (%3%)").arg(it.key())
                            .arg(it.value(), 0, 'f', 2)
                            .arg(percentage, 0, 'f', 1));
        slice->setBrush(QColor(colors[colorIndex % colors.size()]));
        colorIndex++;
    }

    // Create and customize pie chart
    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Expense Distribution");
    pieChart->legend()->setAlignment(Qt::AlignRight);
    pieChart->setBackgroundVisible(false);
    pieChart->setTheme(isDarkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);

    // Create chart view
    expenseChartView = new QChartView(pieChart);
    expenseChartView->setRenderHint(QPainter::Antialiasing);
    expenseChartView->setMinimumHeight(300);
    pieChartLayout->addWidget(expenseChartView);

    // Monthly Income vs Expenses Bar Chart Section
    QGroupBox *barChartGroup = new QGroupBox("Monthly Income vs Expenses");
    QVBoxLayout *barChartLayout = new QVBoxLayout(barChartGroup);

    // Calculate monthly data
    QMap<QString, QPair<double, double>> monthlyData;
    QStringList months;
    for (const Transaction& trans : transactions) {
        QString month = trans.datetime().toString("yyyy-MM");
        if (trans.type() == Transaction::Income) {
            monthlyData[month].first += trans.amount();
        } else {
            monthlyData[month].second += std::abs(trans.amount());
        }
    }

    // Create bar chart
    QBarSeries *barSeries = new QBarSeries();
    QBarSet *incomeSet = new QBarSet("Income");
    QBarSet *expenseSet = new QBarSet("Expenses");

    for (auto it = monthlyData.begin(); it != monthlyData.end(); ++it) {
        months << it.key();
        *incomeSet << it.value().first;
        *expenseSet << it.value().second;
    }

    incomeSet->setColor(QColor("#2ecc71")); // Green for income
    expenseSet->setColor(QColor("#e74c3c")); // Red for expenses
    barSeries->append(incomeSet);
    barSeries->append(expenseSet);

    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Monthly Comparison");
    barChart->setTheme(isDarkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);
    barChart->setBackgroundVisible(false);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    monthlyComparisonChart = new QChartView(barChart);
    monthlyComparisonChart->setRenderHint(QPainter::Antialiasing);
    monthlyComparisonChart->setMinimumHeight(300);
    barChartLayout->addWidget(monthlyComparisonChart);

    // Balance Trend Line Chart Section
    QGroupBox *lineChartGroup = new QGroupBox("Balance Trend");
    QVBoxLayout *lineChartLayout = new QVBoxLayout(lineChartGroup);

    QLineSeries *balanceSeries = new QLineSeries();
    balanceSeries->setName("Balance");
    QPen pen = balanceSeries->pen();
    pen.setWidth(2);
    balanceSeries->setPen(pen);

    // Calculate running balance for line chart
    QVector<Transaction> sortedTransactions = transactions;
    std::sort(sortedTransactions.begin(), sortedTransactions.end(),
              [](const Transaction& a, const Transaction& b) {
                  return a.datetime() < b.datetime();
              });

    double runningBalance = 0;
    for (const Transaction& trans : sortedTransactions) {
        runningBalance += (trans.type() == Transaction::Income ? trans.amount() : -std::abs(trans.amount()));
        balanceSeries->append(trans.datetime().toMSecsSinceEpoch(), runningBalance);
    }

    QChart *lineChart = new QChart();
    lineChart->addSeries(balanceSeries);
    lineChart->setTitle("Balance Over Time");
    lineChart->setTheme(isDarkTheme ? QChart::ChartThemeDark : QChart::ChartThemeLight);
    lineChart->setBackgroundVisible(false);

    QDateTimeAxis *axisX2 = new QDateTimeAxis;
    axisX2->setFormat("MM-dd-yyyy");
    lineChart->addAxis(axisX2, Qt::AlignBottom);
    balanceSeries->attachAxis(axisX2);

    QValueAxis *axisY2 = new QValueAxis;
    lineChart->addAxis(axisY2, Qt::AlignLeft);
    balanceSeries->attachAxis(axisY2);

    balanceTrendChart = new QChartView(lineChart);
    balanceTrendChart->setRenderHint(QPainter::Antialiasing);
    balanceTrendChart->setMinimumHeight(300);
    lineChartLayout->addWidget(balanceTrendChart);

    // Add all widgets to the content layout in order
    contentLayout->addWidget(overviewGroup);
    contentLayout->addWidget(pieChartGroup);
    contentLayout->addWidget(barChartGroup);
    contentLayout->addWidget(lineChartGroup);
    contentLayout->addStretch(); // Add stretch at the end

    // Set the scroll area's widget
    scrollArea->setWidget(scrollContent);

    // Add scroll area to main layout
    mainLayout->addWidget(scrollArea);
}
void MainWindow::updateBalance()
{
    balanceLabel->setText(QString("$%1").arg(currentBalance, 0, 'f', 2));
    incomeLabel->setText(QString("Income: $%1").arg(totalIncome, 0, 'f', 2));
    expenseLabel->setText(QString("Expenses: $%1").arg(totalExpenses, 0, 'f', 2));

    // Update balance label color based on amount
    if (currentBalance > 0) {
        balanceLabel->setStyleSheet("color: #2ecc71;"); // Green for positive
    } else if (currentBalance < 0) {
        balanceLabel->setStyleSheet("color: #e74c3c;"); // Red for negative
    } else {
        balanceLabel->setStyleSheet(""); // Default color for zero
    }
}

void MainWindow::clearTransactionForm()
{
    amountEdit->clear();
    descriptionEdit->clear();
    typeCombo->setCurrentIndex(0);
    categoryCombo->setCurrentIndex(0);
    dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

void MainWindow::updateTransactionTable()
{
    transactionTable->setRowCount(0);  // Clear table

    // Update the table with current transactions
    for (const Transaction& trans : transactions) {
        int row = transactionTable->rowCount();
        transactionTable->insertRow(row);

        QTableWidgetItem *typeItem = new QTableWidgetItem(
            trans.type() == Transaction::Income ? "Income" : "Expense");
        typeItem->setForeground(
            trans.type() == Transaction::Income ? Qt::darkGreen : Qt::red);

        transactionTable->setItem(row, 0, typeItem);
        transactionTable->setItem(row, 1, new QTableWidgetItem(QString::number(std::abs(trans.amount()), 'f', 2)));
        transactionTable->setItem(row, 2, new QTableWidgetItem(trans.description()));
        transactionTable->setItem(row, 3, new QTableWidgetItem(trans.category()));
        transactionTable->setItem(row, 4, new QTableWidgetItem(trans.datetime().toString("yyyy-MM-dd hh:mm")));
    }
}

void MainWindow::searchTransactions()
{
    updateTransactionTable();  // For now, just refresh the table
}
void MainWindow::applyFilters()
{
    updateTransactionTable();  // For now, just refresh the table
}

void MainWindow::clearFilters()
{
    searchEdit->clear();
    categoryFilter->setCurrentIndex(0);
    minAmountFilter->clear();
    maxAmountFilter->clear();
    startDateFilter->setDate(QDate::currentDate().addMonths(-1));
    endDateFilter->setDate(QDate::currentDate());

    updateTransactionTable();
}
void MainWindow::deleteSelectedTransaction()
{
    int row = transactionTable->currentRow();
    if (row < 0) return;  // No row selected

    Transaction trans = transactions[row];

    // Show confirmation dialog
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete",
                                  "Are you sure you want to delete this transaction?\n\n"
                                  "Type: " + QString(trans.type() == Transaction::Income ? "Income" : "Expense") + "\n" +
                                      "Amount: $" + QString::number(std::abs(trans.amount()), 'f', 2) + "\n" +
                                      "Description: " + trans.description(),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Delete from database first
        if (!dbManager.deleteTransaction(
                trans.datetime().toString(Qt::ISODate),
                trans.amount(),
                trans.description())) {
            QMessageBox::critical(this, "Error", "Failed to delete transaction from database!");
            return;
        }

        // If database delete was successful, update totals and UI
        if (trans.type() == Transaction::Income) {
            totalIncome -= std::abs(trans.amount());
            currentBalance -= trans.amount();
        } else {
            totalExpenses -= std::abs(trans.amount());
            currentBalance += std::abs(trans.amount());  // Add back the negative amount
        }

        // Remove from transactions vector
        transactions.remove(row);

        // Update UI
        updateBalance();
        updateTransactionTable();
        updateAnalytics();

        QMessageBox::information(this, "Success", "Transaction deleted successfully!");
    }
}

void MainWindow::handleTransactionTableContextMenu(const QPoint& pos)
{
    QTableWidgetItem *item = transactionTable->itemAt(pos);
    if (item) {
        QMenu contextMenu(tr("Context menu"), this);
        QAction *deleteAction = contextMenu.addAction("Delete Transaction");

        if (contextMenu.exec(transactionTable->viewport()->mapToGlobal(pos)) == deleteAction) {
            deleteSelectedTransaction();
        }
    }
}
// Add these functions to your mainwindow.cpp file



void MainWindow::setupFilters()
{
    QGroupBox *filterGroup = new QGroupBox("Filters");
    QGridLayout *filterLayout = new QGridLayout(filterGroup);

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Search transactions...");

    categoryFilter = new QComboBox;
    categoryFilter->addItems({"All Categories", "Salary", "Food", "Transport",
                              "Entertainment", "Bills", "Shopping", "Other"});

    minAmountFilter = new QLineEdit;
    maxAmountFilter = new QLineEdit;
    minAmountFilter->setPlaceholderText("Min amount");
    maxAmountFilter->setPlaceholderText("Max amount");

    startDateFilter = new QDateEdit(QDate::currentDate().addMonths(-1));
    endDateFilter = new QDateEdit(QDate::currentDate());

    filterLayout->addWidget(new QLabel("Search:"), 0, 0);
    filterLayout->addWidget(searchEdit, 0, 1);
    filterLayout->addWidget(new QLabel("Category:"), 1, 0);
    filterLayout->addWidget(categoryFilter, 1, 1);
    filterLayout->addWidget(new QLabel("Amount Range:"), 2, 0);
    filterLayout->addWidget(minAmountFilter, 2, 1);
    filterLayout->addWidget(maxAmountFilter, 2, 2);
    filterLayout->addWidget(new QLabel("Date Range:"), 3, 0);
    filterLayout->addWidget(startDateFilter, 3, 1);
    filterLayout->addWidget(endDateFilter, 3, 2);

    applyFiltersButton = new QPushButton("Apply Filters");
    clearFiltersButton = new QPushButton("Clear Filters");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(applyFiltersButton);
    buttonLayout->addWidget(clearFiltersButton);
    filterLayout->addLayout(buttonLayout, 4, 0, 1, 3);

    // Connect signals
    connect(applyFiltersButton, &QPushButton::clicked, this, &MainWindow::applyFilters);
    connect(clearFiltersButton, &QPushButton::clicked, this, &MainWindow::clearFilters);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::searchTransactions);

    QVBoxLayout *pageLayout = qobject_cast<QVBoxLayout*>(transactionsPage->layout());
    pageLayout->insertWidget(1, filterGroup);
}

QVector<Transaction> MainWindow::getFilteredTransactions()
{
    QVector<Transaction> filtered;
    QString searchText = searchEdit->text().toLower();
    QString category = categoryFilter->currentText();
    double minAmount = minAmountFilter->text().toDouble();
    double maxAmount = maxAmountFilter->text().toDouble();
    QDate startDate = startDateFilter->date();
    QDate endDate = endDateFilter->date();

    for (const Transaction& trans : transactions) {
        bool matches = true;

        // Search text filter
        if (!searchText.isEmpty()) {
            bool matchesSearch = trans.description().toLower().contains(searchText) ||
                                 trans.category().toLower().contains(searchText);
            if (!matchesSearch) matches = false;
        }

        // Category filter
        if (category != "All Categories" && trans.category() != category) {
            matches = false;
        }

        // Amount filter
        if (!minAmountFilter->text().isEmpty() && std::abs(trans.amount()) < minAmount) {
            matches = false;
        }
        if (!maxAmountFilter->text().isEmpty() && std::abs(trans.amount()) > maxAmount) {
            matches = false;
        }

        // Date filter
        QDate transDate = trans.datetime().date();
        if (transDate < startDate || transDate > endDate) {
            matches = false;
        }

        if (matches) {
            filtered.append(trans);
        }
    }

    return filtered;
}

void MainWindow::cleanupCharts()
{
    if (expenseChartView) {
        delete expenseChartView->chart();
    }
    if (monthlyComparisonChart) {
        delete monthlyComparisonChart->chart();
    }
    if (balanceTrendChart) {
        delete balanceTrendChart->chart();
    }
}
