README.txt - Notes on vector, matrix, quaternions 
02/03/2015
02/07/2015

NOT ALL ROUTINES COMPLETELY TESTED!!!

References that appear in source code comments--

1) Kuipers
Kuipers, Jack B., _Quaternions and Rotation Sequences_, (Princeton University Press, 2002).

2) Diebel 
http://www.astro.rug.nl/software/kapteyn/_downloads/attitude.pdf
Representing Attitude: Euler Angles, Unit Quaternions, and Rotation
Vectors, James Diebel, Stanford University, Stanford, California 94301
Email: diebel@stanford.edu, 20 October 2006

Useful online quaternion calculator for comparing results--
http://www.energid.com/resources/quaternion-calculator/

File .c name is the same as the subroutine.
One file per subroutine (generally!)

Name prefix (generally)--
qd_ quaternion
vd3_ vector


Inline coding, i.e. no multiple dimensioned arrays.

# typedefs--

'D' or 'd' = double precision
'S' or 'd' = single precision

'2' = 2 dimension
'3' = 3 dimension

# Vectors

VS3 = single precision 3 elements (xy)
VD3 = double precision 3 elements (xyz)

# Matrix

VS22 = 2x2 single precision
VD22 = 2x2 single precision

{ f11  f12  f13 }
{ f21  f22  f23 }
{ f31  f32  f33 }

VS33 = 3x3 double precision
VD33 = 3x3 double precision

{ d11  d12  d13 }
{ d21  d22  d23 }
{ d31  d32  d33 }

# Quaterion

QUATD = double precision
d0, d1, d2, d3



