[finance_tracker_report.docx](https://github.com/user-attachments/files/18338878/finance_tracker_report.docx)

Modern Finance Tracker
A modern, feature-rich desktop application for managing personal finances built with Qt 6/5. The application provides an intuitive interface for tracking income and expenses, visualizing financial data, and managing transactions.
![image](https://github.com/user-attachments/assets/075128f9-2acc-42ba-ba08-1fcb8ebe11d2)
![image](https://github.com/user-attachments/assets/c672eccd-2ce6-49eb-a989-86e62234105f)
![image](https://github.com/user-attachments/assets/ada230c1-74fb-4601-8171-6653803fb8fe)
Features

Modern User Interface

Clean and intuitive design
Dark and light theme support
Responsive layout
Card-based dashboard


Transaction Management

Quick transaction entry with customizable categories
Support for both income and expense tracking
Detailed transaction history with search and filter capabilities
Easy transaction deletion and modification


Financial Analytics

Real-time balance tracking
Expense distribution by category (Pie Chart)
Monthly income vs expenses comparison (Bar Chart)
Balance trend analysis (Line Chart)


Data Export

Export transactions to CSV
Generate PDF reports
Excel compatible export


Keyboard Shortcuts

Ctrl + N: Add new transaction
Delete: Delete selected transaction
Ctrl + F: Focus search box
F5: Refresh transaction list


Prerequisites

Qt 6/5
CMake 3.5 or higher
C++17 compatible compiler
Qt Additional Modules:

QtWidgets
QtSql
QtCharts



Installation and Setup

Download the project files
Extract them to your desktop or preferred location
Open Qt Creator
Click on "File" → "Open File or Project"
Navigate to the project folder and select the CMakeLists.txt file
In the Configure Project screen:

Select your preferred kit (Qt 6 recommended)
Leave other settings at their defaults
Click "Configure Project"


Wait for the project to be configured
Click the Run button (green play button) or press Ctrl+R to build and run the application

Note: Make sure you have Qt Creator installed with Qt 6 or Qt 5. You can download it from the official Qt website.
Project Structure
Copymodern-finance-tracker/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.cpp
├── mainwindow.h
├── databasemanager.cpp
├── include/
│   ├── transaction.h
│   └── databasemanager.h
└── README.md
Database Schema
The application uses SQLite for data storage with the following schema:
sqlCopyCREATE TABLE transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type INTEGER NOT NULL,
    amount REAL NOT NULL,
    description TEXT,
    category TEXT,
    datetime TEXT NOT NULL
)
Contributing

Fork the repository
Create a new branch (git checkout -b feature/improvement)
Make your changes
Commit your changes (git commit -am 'Add new feature')
Push to the branch (git push origin feature/improvement)
Create a Pull Request

License
This project is licensed under the MIT License, which means:

✔️ You can freely use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software
✔️ The only requirement is that you include the original copyright notice and license in any copy of the software/source
✔️ The software comes with no warranty of any kind

Full license text should be included in the LICENSE file in the repository.
Example MIT License text for your LICENSE file:
CopyMIT License

Copyright (c) [2025] [MouhssineElBoumshouli]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
Acknowledgments

Qt Framework for providing excellent GUI development tools
Icons and themes inspired by modern design principles
Contributors and supporters of the project

Support
If you encounter any issues or have suggestions for improvements, please open an issue on GitHub.
