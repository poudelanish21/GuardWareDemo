
# Guardware Demo 

A brief demonstration project utilizing windows API, cryptographic Next Generation (CNG) implemented in multiple languages.

Explanation Of Steps Performed In This Project.

In C++ Application:

Step 1: Start.\
Step 2: Get the path where python module is located.\
Step 3: Open python script (utilizing CreateProcessW) and store the handles for closing.\
Step 4: Initialize a named pipe.\
Step 5: Create Asymmetric key pair and Export Public Key With MetaData Info To Named Pipe.(All Performed Utilizing CNG)\
Step 9: Read Pipe Which  contains demoAESKey encrypted with public key and decrypt using private key.\
Step 10: Use the demo password key to encrypt a pseudo randomly generated 32 byte key (Assumed as the session AES key) and send session AES key and all data(data in our case is primes <1_000_000)\
Step 13: Close python application process and handle after waiting.\

In Python Application:\
Step 3: Open and check if all required packages are available else terminate connection and install packages.(A second run will work if you are missing packages).\
Step 6: Wait for and read named pipe connection (first message is assumed to be public key)\
Step 7: Encrypt demoPassword with public key recived through pipe and write the cipher text back to the pipe.\
Step 11: Read all Decrypt first 32 bytes of the cipertext using demoPassword and using the now decrypted session AES key decrypt all the bytes read from the pipe.\
Step 12: Display and wait for the user to view for a short while.
