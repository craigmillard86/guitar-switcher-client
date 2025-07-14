#pragma once

// Pairing status enum and global variable

enum PairingStatus {NOT_PAIRED, PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED};
extern PairingStatus pairingStatus; 