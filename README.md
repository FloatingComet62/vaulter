# Vaulter
A simple program to encrypt and decrypt files / directories. Current only POSIX compliant

Usage:
- vaulter encrypt \<key-path> \<target> \<output file> - Encrypt target, target can be either file or directory
- vaulter decrypt \<key-path> \<target file> \<output directory> - Decrypt target file into output directory, the directory is created if it doesn't exist.
- vaulter gen-key \<output-key-path> - Create a random key
- vaulter help - Display this message
