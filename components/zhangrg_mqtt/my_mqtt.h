#ifndef _MY_MQTT_H_
#define _MY_MQTT_H_

#if 1 //tels.giec.cn
#define CLIENT_CA_CRT													\
"-----BEGIN CERTIFICATE-----\r\n"										\
"MIIB6TCCAVICCQDiQDsebchaFjANBgkqhkiG9w0BAQUFADAyMTAwLgYDVQQKEydU\r\n"	\
"TFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMTkxMDMw\r\n"	\
"MDIxNDE2WhcNMzMwNzA4MDIxNDE2WjBAMScwJQYDVQQKEx5UTFMgUHJvamVjdCBE\r\n"	\
"ZXZpY2UgQ2VydGlmaWNhdGUxFTATBgNVBAMTDHRlbHMuZ2llYy5jbjCBnzANBgkq\r\n"	\
"hkiG9w0BAQEFAAOBjQAwgYkCgYEAqs+78JcxmobC91WlzjKMKVfHxAMUBj4T4qUx\r\n"	\
"h8p6coDYNCuJXPFdAzCc1d3BMdOLL3Ty98GeEyAgvaQvjGJaM2fnHqHEJTDqvW/U\r\n"	\
"915uDam+JQLBmpTVG8U5KvTTdKobxiLbB7Kwvm/2RFhrgphBxQeSUM59zVW3AgjT\r\n"	\
"8n7L+yUCAwEAATANBgkqhkiG9w0BAQUFAAOBgQBhkTbkmQ2Sh6cexco4CdJ7sdwZ\r\n"	\
"iS07A2uqQxBIenkRdYbNr8pC6qcYFOZ33vCszkhqvG7WhgQ4d0nU6Vs7it/7uqFj\r\n"	\
"vwdLCDsg1QrIKUWVSLM3jX6aV+Pis5fY0dzZNzbUYqCud2pn2KLT2DWhyZKTD5a1\r\n"	\
"J9TWo18/T5oZkLsKhg==\r\n"												\
"-----END CERTIFICATE-----\r\n"

#define PRIVATE_KEY_RSA													\
"-----BEGIN RSA PRIVATE KEY-----\r\n"									\
"MIICXAIBAAKBgQCqz7vwlzGahsL3VaXOMowpV8fEAxQGPhPipTGHynpygNg0K4lc\r\n"	\
"8V0DMJzV3cEx04svdPL3wZ4TICC9pC+MYlozZ+ceocQlMOq9b9T3Xm4Nqb4lAsGa\r\n"	\
"lNUbxTkq9NN0qhvGItsHsrC+b/ZEWGuCmEHFB5JQzn3NVbcCCNPyfsv7JQIDAQAB\r\n"	\
"AoGAQx5No0RF4YqCod2twVBrcvNjM8ph2/IYrj5rMAiIBOZMDnPtDfBptA8uEXuJ\r\n"	\
"pqKRVpyFvIUAMFO3ic0yQd+DwXqHYQ1MaNpt1bBjCxbNEEpLx8RsMpsUWNO43MSj\r\n"	\
"Mra2pSvht1sRhBO1EdHcg8/gRZ+Kb30oYFeY8SGDS6s1DwECQQDh02rd8B0zABpQ\r\n"	\
"K1WY0GBTidCKc4Ne3p+LjFk94CRXwmqdJlSA1qP7TRj4e61H8XLU9mR8d4rYn2wc\r\n"	\
"l1Iy3nXNAkEAwaJ/oLPC5Mx+t7ZmvnT3x/feSiO2RgmxMKvwhEqkSLZyKZ+sJhK8\r\n"	\
"UogwolvvRpIdjVmG7S89y/iQA2o5PlJCuQJBANiw66zHCN21TYp76ojV+nhul4Mi\r\n"	\
"0H4qFOHs7H0pJPZpAbCF5TQzweoT0tAsrNuJ3exn+YhNlRfD8hOFSESznkkCQAW5\r\n"	\
"2nBUIIVHjs+78xnat6EQMWCb4b38d2LrEre2xVjpHEMi16Jgl3URVe7XLSI3G+yc\r\n"	\
"IyNMbFVlykxvjbXMFPECQGPC3CdSNeIvJfzXY6tyS0yXH3VNFzljgqPRQugi4KjB\r\n"	\
"eV69+sL4PbkGPsU7PDMEpBUnNM2IWsEV4Rp4Wwb5Zbs=\r\n"						\
"-----END RSA PRIVATE KEY-----\r\n"
#endif

#if 0 //ih.giec.cn
#define CLIENT_CA_CRT 													\
"-----BEGIN CERTIFICATE-----\r\n"										\
"MIIB6TCCAVICCQC8si/5K4ZUlDANBgkqhkiG9w0BAQUFADAyMTAwLgYDVQQKEydU\r\n"	\
"TFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwIBcNMTkwODAx\r\n"	\
"MDI0MDAwWhgPMjE1NjA2MjMwMjQwMDBaMD4xJzAlBgNVBAoTHlRMUyBQcm9qZWN0\r\n"	\
"IERldmljZSBDZXJ0aWZpY2F0ZTETMBEGA1UEAxMKaWguZ2llYy5jbjCBnzANBgkq\r\n"	\
"hkiG9w0BAQEFAAOBjQAwgYkCgYEAxKw0s1xGelGH5k2QduaOAsAkjibrh3zDwDnZ\r\n"	\
"uWyWyScRShs5ciczKlsmAF+26iQf5njipww8gXD/FJyL6SzSrnNZ5CCXG9ezc+jU\r\n"	\
"HWQ5dToVr/ImzS8jciX8p2zB1AHXTX2DdUEtTTGoarT91M5tls0WVQBuBJazWesY\r\n"	\
"KVvjTrcCAwEAATANBgkqhkiG9w0BAQUFAAOBgQDUoDz4qevyomMbUSRBms5tXdUZ\r\n"	\
"DU8AbWPa/GLdeQZoOSCxGsBQqqU0yK+DmTkHg0c3XoZYz2W0LN62aq4p3bQ7hQGa\r\n"	\
"D9DaOZOCml7PfU+1/QvzcOcWOGTq/ClEcR39a+iX3kk6rxGxDHS/IhaCj4Yp0eZU\r\n"	\
"A0csuQ+h6DTDJd1CUw==\r\n"												\
"-----END CERTIFICATE-----\r\n"

#define PRIVATE_KEY_RSA 												\
"-----BEGIN RSA PRIVATE KEY-----\r\n"									\
"MIICXgIBAAKBgQDErDSzXEZ6UYfmTZB25o4CwCSOJuuHfMPAOdm5bJbJJxFKGzly\r\n"	\
"JzMqWyYAX7bqJB/meOKnDDyBcP8UnIvpLNKuc1nkIJcb17Nz6NQdZDl1OhWv8ibN\r\n"	\
"LyNyJfynbMHUAddNfYN1QS1NMahqtP3Uzm2WzRZVAG4ElrNZ6xgpW+NOtwIDAQAB\r\n"	\
"AoGBAKamuT1mqn3JiluXTBvEf39LK26dAiVbjk/bdWeO4TJv/VGLQiE9oDh65feZ\r\n"	\
"I1jB5ZoYww4D611Pz4W6pT+fcKFMvXrOTTCMCKsVDenERRwspBntD63BwJMbtrfD\r\n"	\
"nHzIlCBP2u1P6Jla4AeQtRaBdbgMTNHLIFSb9jYGqjx9SMVxAkEA/2NYqQpSchge\r\n"	\
"j1bvO7teJ9o1oG4jZVVPi+d/IraBH2ufE91nWmZ3KvBMPiN9cnA8X5ycAGUnGHma\r\n"	\
"89Cz9TJLnwJBAMUk2AT/0BubrbjaechDFOtoHy5qiTVWkc8hFeTy1sNrbtCi/hl+\r\n"	\
"sxX54tF/lyAKtwROdWWIwhJ8Btkl1o1BpekCQQCZhDuQaX8DzJHaMjTeoBPKOvZP\r\n"	\
"eMVYLX8Cp2aaFEDsBUJ7q3RsN/ncWtRJ6gWHnAX+p2JzLlrg8IB/nW3/r3sbAkEA\r\n"	\
"hrswWvJNUqDpb/z337JguKVXMgyjKu24Bpuw7+A6gCGLAVFzYQ8i6zD1H4IYUVCh\r\n"	\
"VXMdbj+xippyv7LJV453CQJANEk8H7TQ8IxcT6hVV1BTImC5x6CnGMhv3gOaZtgP\r\n"	\
"iPXR2yTQleubxL552kKh6ufzn4SfnZFuYOInSuQ5UeJ8KQ==\r\n"					\
"-----END RSA PRIVATE KEY-----\r\n"
#endif

void mqtt_app_start(void);

#endif