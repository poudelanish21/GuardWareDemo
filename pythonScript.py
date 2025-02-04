try:
    import os
    import win32file, win32pipe, pywintypes
    import time
    from cryptography.hazmat.primitives.asymmetric import rsa
    from cryptography.hazmat.primitives.asymmetric import padding
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    import struct
except ImportError as e:
    print("Import Error: " + str(e))
    os.system("python -m pip install pywin32 cryptography")
    

server_public_key:bytearray = bytearray()
def parse_rsapublicblob(blob):
    """
    Parse a Microsoft RSAPUBLICBLOB with big-endian exponent (e) and modulus (n).
    """
    # Parse the BCRYPT_RSAKEY_BLOB header
    magic, bit_length, cb_public_exp, cb_modulus = struct.unpack_from("<LLLL", blob, 0)

    # Validate magic number (BCRYPT_RSAPUBLIC_MAGIC)
    if magic != 0x31415352:  # 'RSA1' in ASCII
        raise ValueError("Invalid RSAPUBLICBLOB magic number")

    # Calculate offsets
    offset = 24  # Skip the 24-byte header

    # Extract exponent (BIG-ENDIAN)
    public_exp_bytes = blob[offset : offset + cb_public_exp]
    e = int.from_bytes(public_exp_bytes, byteorder="big")
    offset += cb_public_exp

    # Extract modulus (BIG-ENDIAN)
    modulus_bytes = blob[offset : offset + cb_modulus]
    n = int.from_bytes(modulus_bytes, byteorder="big")

    return n, e

def encrypt_with_rsapublicblob(public_key_blob, data):
    """
    Encrypt data using a Microsoft RSAPUBLICBLOB with big-endian exponent.
    """
    n, e = parse_rsapublicblob(public_key_blob)
    public_key = rsa.RSAPublicNumbers(e, n).public_key()
    encrypted_data = public_key.encrypt(
        data.encode("utf-8"),
        padding=padding.PKCS1v15()
    )
    return encrypted_data
def decrypt(encypted_password:bytes, key:bytes):

    # Default IV (a block of zeros)
    iv = b'\x00' * 16  # AES block size is 16 bytes

    # Create AES cipher in CBC mode
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))

    # Create a decryptor object
    decryptor = cipher.decryptor()

    # Decrypt the ciphertext
    decrypted_data = decryptor.update(encypted_password) + decryptor.finalize()

    plaintext = decrypted_data
    return plaintext
    
try:
    quit = False
    encrypted_message = []
    while not quit:
        try:
            handle = win32file.CreateFileW('\\\\.\\\\pipe\\m_GuardWareDemoPipe', win32file.GENERIC_READ|win32file.GENERIC_WRITE, 0, None, win32file.OPEN_EXISTING, 0, None)
            res = win32pipe.SetNamedPipeHandleState(handle, win32pipe.PIPE_READMODE_MESSAGE, None, None)
            if res == 0:
                print(f"SetNamedPipeHandleState return code: {res}")
            while True:
                if len(server_public_key) == 0:
                    resp = win32file.ReadFile(handle, 4096)
                    print("Server public key received")
                    server_public_key = resp[1]
                    win32file.WriteFile(handle, encrypt_with_rsapublicblob(server_public_key, "mGuardWareDemomGuardWareDemo1111"))
                    print("Client symmentric key sent")
                else:
                    resp = win32file.ReadFile(handle, 4096)
                    encrypted_message.append(resp[1])
        except pywintypes.error as e:
            if e.args[0] == 2:
                #No pipe, trying again in a sec
                time.sleep(1)
            elif e.args[0] == 109:
                print("Pipe closed")
                server_public_key = bytearray()
                quit = True
    password = decrypt(encypted_password=encrypted_message[0][:32], key=b"mGuardWareDemomGuardWareDemo1111")
    print(f"Password: {password}")
    buffer = bytearray()
    for i in range(1, len(encrypted_message)):
        buffer += bytearray(encrypted_message[i])
    print(f"Plaintext: {str(decrypt(buffer, password))}")
    time.sleep(100)
except Exception as Exp:
    print(str(Exp))
    time.sleep(100)
        