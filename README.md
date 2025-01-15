# CSCI 366

This is the root folder for the CSCI 366 class project.

You can find the following content in it:

* `/assembly` - assembly homework and tools
* `/bit-tak-toe` - The bit-tak-toe project
* `/c` - C homework and examples
* `/cpu` - CPU Simulators
* `/lmsm` - The Little Man Stack Machine project
* `/grading` - The grading folder, where the autograder will push your grades up to

## Getting Your Private Copy

Please use the following steps to create a *private* copy of this repo for your work:

- Create a *private* repository in your own account by
    - Going to <https://github.com/new>
    - Enter the name `csci-366-spring2025-private`
    - Select `Private`
    - Navigate to the `Settings` -> `Manage Access` section
    - Add `1cg` as a collaborator

Once your repository is initialized, you can pull it down to your local machine:

```bash
$ git clone <your github repo url>
```

You can find the github repo url when you look at the repository in github.

Next, you should add the class repository as an upstream git repo:

```bash
$ cd csci-366-spring2025-private
$ git remote add upstream https://github.com/msu/csci-366-spring2025.git
$ git pull upstream main
$ git push origin main
```
This will synchronize your private repository with the class repository.

When you want to get an update from the public class repository you can run this command:

```
$ git pull upstream main
``` 

# IDE Setup

This class can be done on any Operating System.  We *strongly* recommend using [CLion](https://www.jetbrains.com/clion/download/)
for developing the project as it has an excellent debugger and test runner.  You will need to apply for a
student licence for Clion [here](https://www.jetbrains.com/shop/eform/students).

Here are the details for each:

## Windows

If you use CLion (you will) the out of the box [mingw](https://www.mingw-w64.org/) system should just work.

## OSX

1. Make sure you have at least 5.27GB of free disk space left on your Mac
3. Open macOS Terminal
4. Install Xcode command line developer tools `xcode-select --install`
5. When prompted to install command line developer tools, click the Install button
6. Install Homebrew package manager `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
7. Install CLion through Homebrew `brew install clion`
8. CLion should configure your toolchain automatically, make sure to navigate to Preferences | Build, Execution, Deployment | Toolchains in CLion and check the default toolchain to make sure there is no warning.
9. Now that you have CLion installed and configured, Head to the CLion welcome screen.
10. Click "Open" that is located near title bar
11. In the pop-up Finder window, navigate to your repo and open it in CLion
12. In CLion right click and load the `CMakeLists.txt` file in the root directory

## Linux

It should all just work.  You need to install CLion and maybe cmake, depending on your distro.

