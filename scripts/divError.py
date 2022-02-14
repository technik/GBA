from math import *
from matplotlib import pyplot as plt

ScreenHeight = 160;
ScreenWidth = 240;
scanlineOffset = ScreenHeight/2;
tgy = 0.5
tgx = tgy*ScreenWidth/ScreenHeight
cz = 384/256.0
# Reference code
def xPosRef(vCount, sx):
	l = cz/(vCount-scanlineOffset+0.5);
	d = l * ScreenHeight;

	pa = l;
	x = -l*ScreenWidth/2
	y = d
	tx = x+pa*sx
	return tx

def xPosFixed(vCount, sx):
	# discretize inputs
	fcz = int(cz*256)/256.0
	fdiv = int(256*266/(int(vCount-scanlineOffset)))/(256*256.0)
	l = int(256*16*fcz*fdiv)/(256*16.0);
	d = l * ScreenHeight;

	pa = int(l*256)/256;
	x = -l*ScreenWidth/2
	y = d
	tx = x+pa*sx
	return tx

def xPosRect(vCount, sx):
	# discretize inputs
	fcz = int(cz*256)/256.0
	fdiv = int(256*266/(int(vCount-scanlineOffset)))/(256*256.0)
	l = int(256*16*fcz*fdiv)/(256*16.0);
	d = l * ScreenHeight;

	pa = int(l*256+0.5)/256;
	x = -l*ScreenWidth/2
	y = d
	tx = x+pa*sx
	return tx

def xPosRect2(vCount, sx):
	# discretize inputs
	fcz = int(cz*256+0.5)/256.0
	fdiv = int(0.5+256*266/(int(vCount-scanlineOffset)))/(256*256.0)
	l = int(256*16*fcz*fdiv+0.5)/(256*16.0);
	d = l * ScreenHeight;

	pa = int(l*256+0.5)/256;
	x = -l*ScreenWidth/2
	y = d
	tx = x+pa*sx
	return tx

sy = range(int(scanlineOffset+1),int(ScreenHeight));
xMin = [xPosRef(y,ScreenWidth/2-0.5) for y in sy]
xMed = [xPosRef(y,ScreenWidth/2) for y in sy]
xMax = [xPosRef(y,ScreenWidth/2+0.5) for y in sy]

xMinFixed = [xPosFixed(y,ScreenWidth/2-1) for y in sy]
xMinRect = [xPosRect(y,ScreenWidth/2-1) for y in sy]
xMinRect2 = [xPosRect2(y,ScreenWidth/2-1) for y in sy]

plt.plot(sy,xMin)
plt.plot(sy,xMed)
plt.plot(sy,xMax)

plt.plot(sy,xMinFixed)
plt.plot(sy,xMinRect)
plt.plot(sy,xMinRect2)

plt.show()