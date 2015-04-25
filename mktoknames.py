toks = {0: '<<EOF>>'}
for line in open('parser.h'):
	_, tokname, value = line.split()
	value = int(value)
	toks[value] = tokname
print toks
arr = ['ERROR'] * (max(toks.keys())+1)
for k, v in toks.iteritems():
	arr[k]=v
print arr
f = open('toknames.c', 'w')
f.write('const char *toknames[] = {\n')
for item in arr:
	f.write('\t"%s",\n'%(item))
f.write('};\n')
f.close()
