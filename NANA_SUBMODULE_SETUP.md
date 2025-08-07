# Nana UI Library - Git Submodule Setup

This document provides instructions for setting up and managing the Nana UI library as a git submodule in the Browser Sanity project.

## What is a Git Submodule?

Git submodules allow you to keep a Git repository as a subdirectory of another Git repository. This lets you clone another repository into your project and keep your commits separate.

## Why Use a Submodule for Nana?

- **Version Control**: Ensures everyone uses the same version of Nana
- **Easier Updates**: Simplifies updating to newer Nana versions
- **Clean Repository**: Keeps the main repository clean without large library code
- **Build Consistency**: Guarantees consistent builds across different development environments

## Initial Setup (Already Completed)

The Nana submodule has been configured in the project with the following settings:

```
[submodule "deps/nana"]
    path = deps/nana
    url = https://github.com/cnjinhao/nana.git
    branch = master
```

## Cloning the Repository with Submodules

When cloning the Browser Sanity repository for the first time:

```bash
# Option 1: Clone with submodules in one command
git clone --recurse-submodules https://github.com/your-repo/browser-sanity.git

# Option 2: Clone normally, then initialize submodules
git clone https://github.com/your-repo/browser-sanity.git
cd browser-sanity
git submodule init
git submodule update
```

## Updating the Nana Submodule

To update the Nana library to the latest version:

```bash
cd deps/nana
git pull origin master
cd ../..
git add deps/nana
git commit -m "Update Nana submodule to latest version"
```

## Switching to a Specific Nana Version

If you need to use a specific version of Nana:

```bash
cd deps/nana
git checkout <tag-or-commit-hash>
cd ../..
git add deps/nana
git commit -m "Switch Nana submodule to version X.Y.Z"
```

## Build Integration

The Visual Studio project file `BrowserSanity_Nana.vcxproj` has been configured to include the Nana source files from the submodule location. The key integration points are:

1. Include directories point to `deps/nana/include`
2. Source files are included from `deps/nana/source`
3. Required preprocessor definitions are set for Nana

## Troubleshooting

If you encounter issues with the Nana submodule:

1. Ensure the submodule is properly initialized: `git submodule status`
2. Try reinitializing: `git submodule update --init --recursive`
3. Check if the correct branch is checked out in the Nana directory
4. Verify that the Visual Studio project is correctly referencing the submodule path

## Additional Resources

- [Git Submodules Documentation](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
- [Nana UI GitHub Repository](https://github.com/cnjinhao/nana)