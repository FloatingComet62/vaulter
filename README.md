## Vaulter

Encrypt files or directories with ease.
Currently only UNIX compliant

### Usage
```bash
vaulter encrypt <key-path> <target> <output-file>
vaulter decrypt <key-path> <target-file> <output-directory>
vaulter gen-key <output-key-path>
vaulter help
```

### Arguments
- `<key-path>`  
  Path to the encryption/decryption key

- `<target>`  
  File or directory to encrypt

- `<target-file>`  
  Encrypted file to decrypt

- `<output-file>`  
  Output file for encrypted data

- `<output-directory>`  
  Directory where decrypted files will be written  
  (created if it does not exist)

- `<output-key-path>`  
  Path where the generated key will be saved

### Options
```bash
-h, --help        Display help message
-V, --version     Show version information
```

### Commands
```bash
encrypt           Encrypt a file or directory
decrypt           Decrypt a file into a directory
gen-key           Generate a random encryption key
help              Display help message
``` vaulter help - Display this message
