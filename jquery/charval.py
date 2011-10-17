import sys
jq = open('jquery.min.js')
print 'const char JQUERY_STR[] = {'
character_vals = [ "%d"%ord(c) for x in jq  for c in x ]
print ','.join(character_vals)
print '};'
