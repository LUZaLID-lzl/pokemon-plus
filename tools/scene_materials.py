"""Shared colour gates for pixel-art material masks."""
def selected(g,r,gc,b):
 if g=='all':return True
 if g=='blueleaf':return b>r*1.12 and b>gc*1.04 and gc>r*1.1 and gc>95
 if g=='leaf':return gc>r*1.05 and gc>b*.82 and gc>34
 if g=='pink':return r>gc*1.17 and r>b*.95 and r>65
 if g=='autumn':return r>gc*1.1 and gc>b*1.12 and r>55
 if g=='snow':return b>r*.95 and gc>r*.93 and b>45
 if g=='water':return b>r*1.17 and gc>r*1.1 and b>40
 if g=='crystal':return b>r*1.14 and b>gc*1.10 and b>65
 if g=='mushroom':return (r>gc*1.3 and r>b*1.3) or (gc>b*1.6 and r>b*1.6 and gc>90)
 if g=='stars':return min(r,gc,b)>45 and max(r,gc,b)>90
 if g=='falls':return b>r*.96 and gc>r*.95 and min(r,gc,b)>75
 return False
