#include <iostream>

std::string Int2String(int u)
{
    if(u == 0) return "0";
    int neg = 0;
    if(u<0) neg = 1, u *= -1;
    std::string s;
    while(u){
        s.push_back(u%10+'0');
        u /= 10;
    }
    std::string ss;
    if(neg) ss.push_back('-');
    for(int i = s.size()-1; i >= 0; i--) ss.push_back(s[i]);
    return ss;
}

std::string Int2String(int u, unsigned int digit)
{
    std::string s;
    for(int i = 0; i < digit; i++){
        s.push_back(u%10+'0');
        u /= 10;
    }
    std::string ss;
    for(int i = s.size()-1; i >= 0; i--) ss.push_back(s[i]);
    return ss;
}

int String2Int(std::string s)
{
    int u = 0;
    int sign = 1;
    for(char c: s){
        if(c == '-') sign *= -1;
        u = u * 10 + (c-'0');
    }
    return sign*u;
}

std::string Float2String(float u, unsigned int digit)
{
    int neg = 0;
    if(u<0) neg = 1, u *= -1;
    int left = u;
    int right = (u - (float)left)*std::pow(10, digit);
    if(neg) left *= -1;
    std::string s = Int2String(left);
    if(digit) s += ('.' + Int2String(right, digit));
    return s;
}

float String2Float(std::string s)
{
    int l = 0, r = s.size()-1;
    int neg = 0;
	if(s[l] == '-') neg = 1, l++;
	int dotPos = r+1;
	for(int i = l; i <= r; i++) if(s[i] == '.') dotPos = i;
	float left = 0.0f, right = 0.0f;
	for(int i = l; i <= dotPos-1; i++){
		left = left * 10.0 + (s[i] - '0');
	}
	for(int i = r; i >= dotPos+1; i--){
		right = right * 0.1 + (s[i] - '0');
	}
	if(!neg) return left + right * 0.1;
	else return -(left + right * 0.1);
}

void CopyStringToBuffer(std::string s, char* buffer)
{
    int ptr = 0;
    for(char c: s) buffer[ptr++] = c;
    buffer[ptr] = '\0';
}