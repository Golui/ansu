import math
import copy
import pystache as ps
import itertools
import argparse
from collections import Counter

# Adapted from https://github.com/JarekDuda/AsymmetricNumeralSystemsToolkit/blob/master/ANStoolkit.cpp


def mask(l):
    return (1 << l) - 1


class ANS:

    def __init__(self, raw_probabilities, *, table_size_log=16):
        self.rprob = raw_probabilities
        self.tsl = table_size_log
        self.quantize = self._quantize_fast
        self.spread = self._spread_fast

        self.quantize()
        self.spread()
        self.create_tables()

    @property
    def ts(self):
        return 1 << self.tsl

    @property
    def allen(self):
        return len(self.rprob)

    def _quantize_fast(self):
        used = 0
        maxv = 0
        self.q = [0]*self.allen
        for i, p in enumerate(self.rprob):
            cur_q = int(self.ts*p)
            if cur_q == 0:
                cur_q += 1
            used += cur_q
            self.q[i] = cur_q
            if p > maxv:
                maxv = p
                maxp = i
        self.q[maxp] += self.ts - used

    # Spread symbols and create nbits.
    def _spread_fast(self):
        pos = 0
        step = (self.ts >> 1) + (self.ts >> 3) + 3
        mask = self.ts - 1
        self.states = [0] * self.ts

        for quant_i, quant in enumerate(self.q):
            for quan_states in range(quant):
                self.states[pos] = quant_i
                pos = (pos + step) & mask

    def ks(self, s):
        return self.tsl - int(math.floor(math.log2(self.q[s])))

    def nb(self, s):
        return (self.ks(s) << (self.tsl + 1)) - (self.q[s] << self.ks(s))

    def create_tables(self):

        self.start = [-s + sum(self.q[:si])
                      for si, s in enumerate(self.q)]

        self.adj_start = [s % self.ts for s in self.start]

        next_ = copy.deepcopy(self.q)

        self.encoding_table = [0] * self.ts
        self.nb_bits = [0] * self.ts
        self.new_x = [0] * self.ts
        for state, symbol in enumerate(self.states):
            # Encoding
            self.encoding_table[self.start[symbol]
                                + next_[symbol]] = self.ts + state

            x = next_[symbol]
            self.nb_bits[state] = self.tsl - int(math.floor(math.log2(x)))
            self.new_x[state] = (x << self.nb_bits[state]) - self.ts
            next_[symbol] += 1

    @staticmethod
    def output(x, bits):
        return format(x & mask(bits), f"0{bits}b")

    def encode_single(self, x, sindex):
        nb_bits = (x + self.nb(sindex)) >> (self.tsl + 1)
        bits = ANS.output(x, nb_bits)
        x = self.encoding_table[self.start[sindex] + (x >> nb_bits)]
        return (x, bits)

    def encode(self, message, symbols=None):
        if symbols is None:
            symbols = sorted(set(message))

        out = ""
        # x, _ = self.encode_single(0, symbols.index(message[0]))
        x = self.encoding_table[0]
        for s in message:
            sindex = symbols.index(s)
            x, bits = self.encode_single(x, sindex)
            out += bits
        # out += ANS.output(x - self.ts, self.tsl)
        return (x - self.ts, out)

    def decode(self, encoded, symbols=None):
        x, stream = encoded
        if symbols is None:
            symbols = [str(i) for i in range(self.allen)]

        out = []
        while len(stream) > 0:
            nb_bits, new_x = self.nb_bits[x], self.new_x[x]
            out.append(symbols[self.states[x]])
            x = new_x + int(stream[-nb_bits:], 2)
            stream = stream[:-nb_bits]
        return out[::-1]


def pick_datatype(val):
    datatypes = ["uint8_t", "uint16_t", "uint32_t", "uint64_t", None]
    dtsel = [8, 16, 32, 64, math.inf]
    datatype = next(i for i, x in zip(
        datatypes, dtsel) if x > math.log2(val))
    if datatype is None:
        print("Too large number")
    return datatype


def comma_sep(what):
    return ", ".join(str(x) for x in what)


def cify(ans):
    renderer = ps.Renderer()
    rendered_h = renderer.render_path("template/ans_table.hpp.stache", {
        "ans": ans,
        "message_dt": pick_datatype(ans.allen - 1),
        "state_dt": pick_datatype(max(ans.encoding_table)),
        "state_delta_dt": pick_datatype(2 * max(ans.encoding_table) - 1)[1:],
        "nb_bits_dt": pick_datatype(max(ans.nb_bits)),
        "nb_dt": pick_datatype(max(ans.nb(x) for x in ans.states)),
        "states": comma_sep(ans.states),
        "new_x": comma_sep(ans.new_x),
        "encoding_table": comma_sep(ans.encoding_table),
        "nb_bits": comma_sep(ans.nb_bits),
        "nb": comma_sep(ans.nb(x) for x in range(ans.allen)),
        "start": comma_sep(ans.start),
        "adj_start": comma_sep(ans.adj_start)
    })

    with open("out/ans_table.hpp", "w") as f:
        f.write(rendered_h)


def process_sample_file(filenm):
    with open(filenm, 'rb') as f:
        MESSAGE = f.read()
        countDict = Counter(MESSAGE)
        countDict.update({i: 0 for i in range(256)})
        symbols, occurences = zip(*sorted(countDict.items()))
        return occurences2prob(*occurences), symbols, MESSAGE


def occurences2prob(*args):
    s = sum(args)
    return [x/s for x in args]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument("-p", type=int, default=5)

    args = parser.parse_args()

    if "file" in args:
        occurences, SYMBOLS, MESSAGE = process_sample_file(args.file)
    else:
        occurences = occurences2prob(10, 10, 12)
        SYMBOLS = ['0', '1', '2']
        MESSAGE = "1102010120"

    ans = ANS(occurences, table_size_log=args.p)
    # print(ans.q)
    # print(ans.states)
    # print(ans.encoding_table)
    # print(ans.start)
    # print([ans.nb(s) for s in range(3)])
    # print(MESSAGE)
    encoded = ans.encode([ord(i) for i in "aaaaaaaaaaaaaaaa"], SYMBOLS)
    # print(encoded)
    # print(*(chr(i) for i in ans.decode(encoded, SYMBOLS)), sep="")
    cify(ans)


if __name__ == '__main__':
    main()
