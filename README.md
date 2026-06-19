# Image Steganography GUI (C + WinAPI)

A Windows GUI application written in C that allows users to hide text messages inside PNG images and extract hidden messages from encoded images using LSB (Least Significant Bit) steganography.

## Features

* Hide text messages inside PNG images
* Extract hidden messages from encoded PNG images
* Native Windows GUI (WinAPI)
* High-capacity storage using RGB channels
* Supports RGB and RGBA PNG images
* Built with C and libpng
* Generates a standalone Windows executable

---

## Requirements

### Option 1: MSYS2 (Recommended)

Install MSYS2:

https://www.msys2.org/

Open the **MSYS2 MinGW 64-bit** terminal and install the required packages:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-libpng
```

Verify installation:

```bash
gcc --version
```

---

## Project Structure

```text
project/
│
├── main.c
├── README.md
└── (generated)
    └── StegGUI.exe
```

---

## Building the Executable

Navigate to the project directory:

```bash
cd /c/path/to/project
```

Compile the project:

```bash
gcc -o StegGUI.exe main.c -lpng -lgdi32 -lcomdlg32
```

If the compilation succeeds, a new executable named:

```text
StegGUI.exe
```

will be created in the current directory.

---

## Running

Launch the application:

```bash
./StegGUI.exe
```

Or simply double-click the executable from Windows Explorer.

---

## Usage

### Hide a Message

1. Click **Select File**
2. Choose a PNG image
3. Enter your message in the text box
4. Click **Hide Message**
5. An encoded PNG file will be generated

### Extract a Message

1. Click **Select File**
2. Choose an encoded PNG image
3. Click **Extract Message**
4. The hidden text will appear in the text box

---

## Supported Formats

| Format   | Supported |
| -------- | --------- |
| PNG      | Yes       |
| JPG/JPEG | No        |
| BMP      | No        |

PNG is required because JPEG compression destroys hidden data.

---

## Notes

* The larger the image, the more text it can store.
* RGB channels are used to maximize storage capacity.
* Hidden messages are not encrypted by default.
* For sensitive data, consider adding AES encryption before embedding.

---

## Troubleshooting

### gcc: command not found

Install GCC:

```bash
pacman -S mingw-w64-x86_64-gcc
```

### png.h: No such file or directory

Install libpng:

```bash
pacman -S mingw-w64-x86_64-libpng
```

### Undefined reference to png_*

Make sure you compile with:

```bash
-lpng
```

---

## License

This project is provided for educational and research purposes.
Use responsibly.
