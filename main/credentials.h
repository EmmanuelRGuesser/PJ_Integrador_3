#ifndef CREDENTIALS_H
#define CREDENTIALS_H

extern char username_ha[32];
extern char password_ha[32];

void load_credentials();
void reset_credentials();

#endif // CREDENTIALS_H