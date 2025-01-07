#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> //Create a window that would hold everything
#include <QPushButton> //Create those clickable buttons
#include <QStackedWidget> //Create sub-windows, show different under different buttons
#include <QLabel> // Writing we you can't edit
#include <QLineEdit> //Single line box where user can input stuff, like the amount or description
#include <QComboBox>
#include <QDateTimeEdit>
#include <QTableWidget>
#include <QVector>
#include <QShortcut>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

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

#include "transaction.h"
#include "databasemanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleTheme();
    void showDashboard();
    void showTransactions();
    void showAnalytics();
    void addNewTransaction();
    void updateBalance();
    void clearTransactionForm();
    void updateTransactionTable();
    void searchTransactions();
    void applyFilters();
    void clearFilters();
    void deleteSelectedTransaction();
    void handleTransactionTableContextMenu(const QPoint& pos);
    void showShortcutsDialog();
    void newTransactionShortcutTriggered();
    void focusSearchBox();
    void exportToCSV();
    void exportToPDF();
    void exportToExcel();
    void updateAnalytics();

private:
    DatabaseManager dbManager;

    // Charts
    QChartView *expenseChartView;
    QChartView *monthlyComparisonChart;
    QChartView *balanceTrendChart;

    // Navigation buttons
    QPushButton *dashboardButton;
    QPushButton *transactionsButton;
    QPushButton *analyticsButton;
    QPushButton *themeButton;

    // Page container
    QStackedWidget *pageStack;

    // Pages
    QWidget *dashboardPage;
    QWidget *transactionsPage;
    QWidget *analyticsPage;

    // Transaction form elements
    QComboBox *typeCombo;
    QLineEdit *amountEdit;
    QLineEdit *descriptionEdit;
    QComboBox *categoryCombo;
    QDateTimeEdit *dateTimeEdit;
    QPushButton *addButton;
    QPushButton *clearButton;

    // Transaction table
    QTableWidget *transactionTable;

    // Balance labels
    QLabel *balanceLabel;
    QLabel *incomeLabel;
    QLabel *expenseLabel;

    // Search and Filter elements
    QLineEdit *searchEdit;
    QComboBox *categoryFilter;
    QDateEdit *startDateFilter;
    QDateEdit *endDateFilter;
    QLineEdit *minAmountFilter;
    QLineEdit *maxAmountFilter;
    QPushButton *applyFiltersButton;
    QPushButton *clearFiltersButton;

    // Keyboard shortcuts
    QShortcut *newTransactionShortcut;
    QShortcut *deleteTransactionShortcut;
    QShortcut *searchShortcut;
    QShortcut *refreshShortcut;

    // Data
    QVector<Transaction> transactions;
    double currentBalance;
    double totalIncome;
    double totalExpenses;

    // Private methods
    void setupUI();
    void setupNavigation();
    void setupDashboard();
    void setupTransactionsPage();
    void setupTransactionForm();
    void setupTransactionTable();
    void setupAnalyticsPage();
    void setupFilters();
    void setTheme(bool darkTheme);
    void cleanupCharts();
    void loadTransactionsFromDatabase();
    QVector<Transaction> getFilteredTransactions();
    bool isDarkTheme = false;
};

#endif
