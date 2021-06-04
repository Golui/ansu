import math
import copy
import pystache as ps
import dill as pickle
import argparse
from collections import Counter
import mmap
import struct
import tempfile
import os

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
		x, _ = self.encode_single(1024, symbols.index(message[0]))
		# x = self.encoding_table[0]
		for s in message:
			sindex = symbols.index(s)
			oldx = x
			x, bits = self.encode_single(x, sindex)
			out += bits
			print(oldx, x, len(bits))
		print("==============")
		return (x - self.ts, out)

	def decode(self, encoded, symbols=None):
		x, stream = encoded
		if symbols is None:
			symbols = [str(i) for i in range(self.allen)]

		print(x + self.ts)
		out = []
		while len(stream) > 0:
			nb_bits, new_x = self.nb_bits[x], self.new_x[x]
			out.append(symbols[self.states[x]])
			oldX = x
			x = new_x + int(stream[-nb_bits:], 2)
			print(oldX + self.ts, x + self.ts, nb_bits, stream[-nb_bits:])
			stream = stream[:-nb_bits]
		return out[::-1]

	def binary_decode(self, data, meta, out_name):
		types = ["B", "H", "I", "L", None]

		sel = dtsel(max(self.encoding_table))
		state_t_size = 2 ** sel
		dt_symbol = types[sel]
		if dt_symbol is None:
			print("Invalid ANS object")
			return

		meta.seek(-(2*state_t_size + 12), 2)
		x, = struct.unpack(dt_symbol, meta.read(state_t_size))

		meta.seek(-8, 2)
		dead_bits, = struct.unpack("B", meta.read(1))
		meta.seek(-4, 2)
		dropBytes, = struct.unpack("I", meta.read(4))
		dropBytes += 1

		nb_bits, new_x = self.nb_bits[x], self.new_x[x]

		with tempfile.NamedTemporaryFile() as out:
			file = mmap.mmap(data.fileno(), 0, prot=mmap.PROT_READ)
			stream = ""
			writebuf = None
			# Loop until "Beginning of file"
			for i in range(0, file.size(), state_t_size)[::-1]:
				cur_val, = struct.unpack(dt_symbol, file[i:i + state_t_size])
				stream = ANS.output(cur_val, state_t_size*8) + stream
				if dead_bits > 0:
					stream = stream[:-dead_bits]
					dead_bits = 0
				while len(stream) >= nb_bits:
					if dropBytes > 0:
						dropBytes -= 1
					else:
						if writebuf is not None:
							out.write(struct.pack("B", writebuf))
					x = new_x + int(stream[-nb_bits:], 2)
					if dropBytes == 0:
						writebuf = self.states[x]
					stream = stream[:-nb_bits]
					nb_bits, new_x = self.nb_bits[x], self.new_x[x]
			out.flush()
			os.system(
				f"< {out.name} xxd -p -c1 | tac | xxd -p -r > {out_name}")


def dtsel(val):
	dtsel = [8, 16, 32, 64, math.inf]
	datatype = next(i for i, x in enumerate(dtsel) if x > math.log2(val))
	return datatype


def pick_datatype(val):
	datatypes = ["uint8_t", "uint16_t", "uint32_t", "uint64_t", None]
	datatype = datatypes[dtsel(val)]
	if datatype is None:
		print("Too large number")
	return datatype


def comma_sep(what):
	return ", ".join(str(x) for x in what)


def cify(ans, out):
	dn = os.path.dirname(__file__)
	templn = os.path.join(dn, "template/ans_table.hpp.stache")

	renderer = ps.Renderer()
	rendered_h = renderer.render_path(templn, {
		"ans": ans,
		"message_dt": pick_datatype(ans.allen - 1),
		"state_dt": pick_datatype(max(ans.encoding_table)),
		# Evil hack to get signed type
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

	with open(out, "w") as f:
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


def generate_c(args):
	if args.file is None:
		if args.d is None:
			print("Specify either a file or the -d option.")
			return
		with open(args.d, "rb") as f:
			ans = pickle.load(f)
	else:
		occurences, SYMBOLS, MESSAGE = process_sample_file(args.file)
		print(occurences)
		ans = ANS(occurences, table_size_log=args.p)

	if args.o is not None:
		cify(ans, args.o)
	else:
		if args.d is None:
			print("Warning: both -o and -d are not specified, nothing will"
				  " be saved!")

	if args.d is not None:
		with open(args.d, "wb") as f:
			pickle.dump(ans, f)
		print(f"Dumped ans to {args.d}")


def decode_file(args):
	with open(args.ans, "rb") as f:
		ans = pickle.load(f)
		with open(args.data, "rb") as d, open(args.meta, "rb") as m:
			ans.binary_decode(d, m, args.out)

def main():
	parser = argparse.ArgumentParser(description="ANS helper script.")
	subp = parser.add_subparsers(title="Available subcommands:")

	generate = subp.add_parser(
		"generate",
		help="Generate a C header"
		" with ANS tables, based on a sample file.")

	generate.set_defaults(func=generate_c)
	generate.add_argument("file", help="The sample file to use."
						  " The occurence of each byte will be tallied."
						  " Based on these, the ANS table will be created."
						  " One can also not specify this argument, and use"
						  " a preexisting ANS object. See -d for details.",
						  nargs='?', default=None)

	generate.add_argument(
		"-o", help="Where to output the C header to.")
	generate.add_argument("-d", help="ANS object file name. Save/Load.")

	generate.add_argument("-p", type=int, default=8,
						  help="Log2 of the target table size. Default 8.")

	decompress = subp.add_parser(
		"decompress",
		help="Decompress an ANSU compressed file.")

	decompress.set_defaults(func=decode_file)
	decompress.add_argument(
		"ans", help="The ANS decompressor object to use")
	decompress.add_argument("data", help="The data file to decompress")
	decompress.add_argument(
		"meta", help="The accompanying meta file to decompress")

	decompress.add_argument(
		"out", help="Where to save the decompressed data")

	args = parser.parse_args()
	args.func(args)


if __name__ == '__main__':
	main()
