#!/usr/bin/env python3

import sys


def generate_mask(s):
	o = ""
	s = s.replace("_", "")
	for c in s:
		if c == '0' or c == '1':
			o += '1'
		else:
			o += '0'
	return int(o, 2)


def generate_match(s):
	o = ""
	s = s.replace("_", "")
	for c in s:
		if c == '0' or c == '1':
			o += c
		else:
			o += '0'
	return int(o, 2)


def find_bits(s):
	bit_start = dict()
	bit_end = dict()
	cur = None
	s += '0' # for case when s ends with bits
	for x, c in enumerate(s):
		if cur == c:
			continue

		if cur and cur not in ['0', '1']:
			bit_end[cur] = x
		cur = c
		if cur in bit_start:
			raise Exception("non-continuous bit in {}".format(s))
		if cur not in ['0', '1']:
			bit_start[cur] = x

	out = [] # out is (bit, start, len)
	for bit in bit_start:
		out.append((bit, bit_start[bit], bit_end[bit] - bit_start[bit]))

	return out


def main():
	with open(sys.argv[1], "r") as fin:
		data = fin.read()
	descs = data.split("\n\n\n")
	gen = ""
	table = ""
	for desc in descs:
		desc = desc.split("\n")
		code = desc[0].split(" ")

		# match
		mask = generate_mask(code[0])
		match = generate_match(code[0])

		# bits decode
		code = "".join([s[::-1] for s in code]).replace("_", "")
		bits = find_bits(code)
		decode_bits = ""
		for bit, start, sz in bits:
			decode_bits += "\tuint32_t {} = GetBits(insn, {}, {});\n".format(bit, start, sz)

		# vars
		x = 0
		get_vars = ""
		for line in desc[1:]:
			x += 1
			if "{" in line:
				break
			line = line.replace(" ", "").replace("_", "")
			var_name, stmt = line.split("=")
			parts = stmt.split("||")[::-1]
			off = 0
			out = []
			for part in parts:
				if part[0] in ['0', '1']:
					var = "0b{}".format(part)
				else:
					var = part[0]
				out.append("({} << {})".format(var, off))
				off += len(part)
			get_vars += "\tuint32_t {} = {};\n".format(var_name, " | ".join(out))

		# body
		func_name = "i_" + desc[x].split(" {")[0]
		func_body = ""
		for line in desc[x + 1:]:
			if "}" in line:
				break
			func_body += line + "\n"

		# generate decode
		sz = len(code) // 8
		table += "\t{ "
		table += "{}, {}, {}, {}".format(sz, bin(mask), bin(match), func_name)
		table += " },\n"

		# generate impl
		gen += "INSN(%s) {\n" % func_name
		gen += decode_bits
		gen += get_vars
		gen += func_body
		gen += "}\n\n"

	with open(sys.argv[2], "w") as fout:
		fout.write(gen + "\n")
		fout.write("insn_t insn_table[] = {\n" + table + "};\n")


if __name__ == "__main__":
	main()
