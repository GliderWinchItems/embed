char fstr[80];
float num = 2.55f;
int m = log10(num);
int digit;
float tolerance = .0001f;

while (num > 0 + precision)
{
    float weight = pow(10.0f, m);
    digit = floor(num / weight);
    num -= (digit*weight);
    *(fstr++)= '0' + digit;
    if (m == 0)
        *(fstr++) = '.';
    m--;
}
*(fstr) = '\0';

