from aes import aes
from random import randint
from aes.utils import int2arr8bit
# needed for our aes_modified patch
from aes.utils import arr8bit2int
from aes.core.para import STATE_LEN
#######

from typing import List, Literal

AES_BLOCK_SIZE_BITS: Literal[128] = 128
AES_BLOCK_SIZE: Literal[16] = 16

class aes_modified(aes):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    
    def enc(self, plaintexts:list, verbose=False):
        # size
        s = len(plaintexts)
        p = (s % STATE_LEN)

        if verbose:
            try: from tqdm import tqdm
            except:
                import warnings
                warnings.warn("can't find module 'tqdm'")
                forloop = range(s//STATE_LEN)
            else:
                forloop = tqdm(range(s//STATE_LEN))
        else: forloop = range(s//STATE_LEN)

        # mode of operation setting
        Mode = self.mode(self.mk, self.iv)
        ciphertexts, i = [], -1

        # encryption
        for i in forloop:
            pt = plaintexts[STATE_LEN*i:STATE_LEN*(i+1)]
            ct = Mode.enc(pt)
            ciphertexts = ciphertexts + ct

        # padding
        # format
        if i==-1:
            pt = plaintexts
        else:
            pt = plaintexts[STATE_LEN*(i+1):]
        
        pt = int2arr8bit(arr8bit2int(pt), p)
        # padding
        pt = self.padding(pt, STATE_LEN, inv=True) # this is the only line of code changed. required for compatibility with the buoy implementation of AES -----------------------------------------------------------------------------------------------------------
        # encryption
        ct = Mode.enc(pt)
        ciphertexts = ciphertexts + ct

        return ciphertexts

    def dec(self, ciphertexts, verbose=False):
        # size
        s = len(ciphertexts)
        assert (s % STATE_LEN)==0

        if verbose:
            try: from tqdm import tqdm
            except:
                import warnings
                warnings.warn("can't find module 'tqdm'")
                forloop = range((s//STATE_LEN)-1)
            else:
                forloop = tqdm(range((s//STATE_LEN)-1))
        else: forloop = range((s//STATE_LEN)-1)

        # mode of operation setting
        Mode = self.mode(self.mk, self.iv)
        plaintexts = []

        # encryption
        for i in forloop:
            ct = ciphertexts[STATE_LEN*i:STATE_LEN*(i+1)]
            pt = Mode.dec(ct)
            plaintexts = plaintexts + pt

        # unpadding
        ct = ciphertexts[-16:]
        pt = Mode.dec(ct)
        pt = self.padding(pt, STATE_LEN, unpad=True, inv=True) # this is the only line of code changed
        plaintexts = plaintexts + pt

        return plaintexts

def bytes_to_list(bytes_input: bytes) -> List[int]:
    return [int(x) for x in bytes_input]

class Encryption(aes_modified):
    def __init__(self, key: List[int]) -> None:
        assert len(key) == AES_BLOCK_SIZE, "Key size must be equal to the block size"
        super().__init__(key, keysize=AES_BLOCK_SIZE_BITS, mode='CBC', padding='PKCS#7', iv=0)

    def encrypt(self, input: bytes, iv: int) -> bytes:
        self._set_ivl(iv)
        return bytes(self.enc(bytes_to_list(input)))
    
    def decrypt(self, input: bytes, iv: int) -> bytes:
        assert (len(input) % AES_BLOCK_SIZE) == 0, "Input size must be a multiple of the AES block size"
        self._set_ivl(iv)
        return bytes(self.dec(bytes_to_list(input)))
    
    def generate_random_iv(self) -> int:
        return randint(0, 0xFFFF) # return a random uint16_t

    def _set_ivl(self, iv: int) -> None:
        self.iv = int2arr8bit(iv, AES_BLOCK_SIZE)[::-1] # reversed

if __name__ == "__main__":
    encryption_key = [0x41] * AES_BLOCK_SIZE
    iv = 1337

    encryption = Encryption(encryption_key)

    print(encryption.encrypt(b"encrypt this", iv))
    print(encryption.decrypt(encryption.encrypt(b"encrypt this", iv), iv).decode())