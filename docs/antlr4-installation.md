# ANTLR4 Installation on Ubuntu

This guide covers installing ANTLR4 for the ifcc compiler on Ubuntu (including WSL).

## Prerequisites

Install the required dependencies:

```bash
sudo apt update
sudo apt install default-jdk wget unzip cmake uuid-dev g++ pkg-config
```

These packages provide:
- **default-jdk** — Java Runtime (required for ANTLR)
- **wget** — Download files
- **unzip** — Extract archives
- **cmake** — Build system for ANTLR C++ runtime
- **uuid-dev** — UUID library (dependency)
- **g++** — C++ compiler
- **pkg-config** — Library configuration tool

## Install ANTLR4 JAR

1. Create the installation directory:

   ```bash
   mkdir -p /home/$USER/antlr4-install
   cd /home/$USER/antlr4-install
   ```

2. Download the ANTLR4 JAR:

   ```bash
   wget https://www.antlr.org/download/antlr-4.13.2-complete.jar
   ```

3. Create the `antlr4` command wrapper:

   ```bash
   cat > ~/antlr4 << 'EOF'
   #!/bin/bash
   export CLASSPATH=".:/home/$USER/antlr4-install/antlr-4.13.2-complete.jar:$CLASSPATH"
   java -jar /home/$USER/antlr4-install/antlr-4.13.2-complete.jar "$@"
   EOF
   chmod +x ~/antlr4
   sudo mv ~/antlr4 /usr/local/bin/
   ```

4. Verify the installation:

   ```bash
   antlr4 -version
   ```

   Add the following to your `~/.bashrc` if needed:

   ```bash
   export CLASSPATH=".:/home/$USER/antlr4-install/antlr-4.13.2-complete.jar:$CLASSPATH"
   ```

## Install ANTLR4 C++ Runtime

1. Download the source:

   ```bash
   cd ~
   wget https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip
   unzip antlr4-cpp-runtime-4.13.2-source.zip
   cd antlr4-cpp-runtime-4.13.2-source
   ```

2. Build and install:

   ```bash
   mkdir build
   cd build
   cmake .. -DANTLR_JAR_LOCATION=/home/$USER/antlr4-install/antlr-4.13.2-complete.jar -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_DEMO=False
   make -j$(nproc)
   sudo make install
   sudo ldconfig
   ```

## Configuration Paths

After installation, note these paths for the compiler configuration:

| Variable | Path |
|----------|------|
| `ANTLRJAR` | `/home/$USER/antlr4-install/antlr-4.13.2-complete.jar` |
| `ANTLRINC` | `/usr/local/include/antlr4-runtime/` |
| `ANTLRLIB` | `/usr/local/lib/libantlr4-runtime.a` |

## Verify Installation

```bash
# Check headers
ls /usr/local/include/antlr4-runtime/

# Check library
ls /usr/local/lib/libantlr4-runtime*

# Save paths for reference
echo "ANTLRJAR=/home/$USER/antlr4-install/antlr-4.13.2-complete.jar
ANTLRINC=/usr/local/include/antlr4-runtime/
ANTLRLIB=/usr/local/lib/libantlr4-runtime.a" > ~/antlr4.paths
```

## Ubuntu 24.04+ Note

On Ubuntu 24.04 and later, avoid installing ANTLR4 via apt if the version is incompatible. Use the manual installation steps above instead.

## Troubleshooting

### JAR not found

Ensure `ANTLRJAR` points to the correct path. The exact path depends on your username:

```bash
# Verify the file exists
ls -la /home/$USER/antlr4-install/
```

### Headers not found

Verify the include path:

```bash
ls /usr/local/include/antlr4-runtime/
```

If headers are in a different location, find them:

```bash
find /usr -name "antlr4-runtime.h" 2>/dev/null
```

### Linking errors

Ensure the library is in the linker path:

```bash
sudo ldconfig
```

### Java not found

If Java is not found, install it:

```bash
sudo apt install default-jdk
java -version
```

## Next Steps

After installing ANTLR4, proceed to [Build Instructions](build.md).
