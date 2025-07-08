# Descartes Builder - Windows Installer Instructions

This readme outlines the steps to build a Windows installer for the **Descartes Builder** application, including a bundled embedded Python.

---

## Steps to Create the Installer

### 1. Clone the Repository

* Clone a fresh copy of the public repository: https://github.com/CPS-research-group/descartes-builder
The following steps to be taken from the new repo.

### 2. Set Up Embedded Python

* Run the following BAT script to set up a portable, embedded version of Python (3.10.8): setup_python_win.bat

This downloads and configures embedded Python inside the project directory.
The Python runtime will be bundled with the application — no need for a global Python install.

### 3. Build the Windows Installer (with Inno Setup)

* Install the Inno Setup software from : https://jrsoftware.org/download.php/is.exe?site=1
Using Inno Setup Compiler, compile the install file: win_installer.iss
This creates a self-contained .exe installer for Descartes Builder. 
This .exe can be shared to have it install the Descartes Builder on a Windows setup.

### 4. Install the application

* Follow the usual steps of installating the application into a desired folder, and the app is ready to use.

### After installation:

* The Descartes Builder app would be installed on the system. Python is bundled inside the app directory, and so no changes are made to the system's Python or environment variables. Your app is now ready to run offline and without interfering with any global Python installation.

* In case, there is some path mismatches, if the application cannot find the embedded python path, it will look for the system python path, and try to continue operation.  