#ifdef __cplusplus
extern "C" {
#endif

typedef struct gcry_ac_key *gcry_ac_key_t;

// Generic
void InitCrypt (void);

// RSA Functions
int GenerateKeyRSA (gcry_ac_key_t *pkeyPublic, gcry_ac_key_t *pkeySecret,
					int nBits);
int EncryptRSA (gcry_ac_key_t keyPublic1, const void *pDataIn, size_t sizeDataIn,
				void *pDataOut, size_t sizeDataOut, size_t *psizeWritten);
int DecryptRSA (gcry_ac_key_t keySecret, const void *pDataIn, size_t sizeDataIn,
				void *pDataOut, size_t sizeDataOut, size_t *psizeWritten);
void PublicKeyToBinaryRSA (gcry_ac_key_t key, void *pDataOut,
						   size_t sizeDataOut, size_t *psizeWritten);
void SecretKeyToBinaryRSA (gcry_ac_key_t key, void *pDataOut,
						   size_t sizeDataOut, size_t *psizeWritten);
int BinaryToKeyRSA (gcry_ac_key_t *pkey, unsigned char *pData, int nDataLen);
void DestroyKey (gcry_ac_key_t key);
void KeyToHexStringRSA (gcry_ac_key_t key, char *pOutStr, size_t sizeOutStr);

// AES Functions
int EncryptAES (const char *sPlainText, int nPlainTextLen,
				const char *sKey, int nKeyLen, char *sOut, int nOutLen);
int DecryptAES (const char *sEncText, int nEncTextLen, const char *sKey,
				int nKeyLen, char *sOut, int nOutLen);

#ifdef __cplusplus
}
#endif

