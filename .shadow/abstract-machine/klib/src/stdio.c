#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void printint(int val)
{
    if(val == 0)
        return;
    printint(val/10);
    putch((val%10)+'0');
    return;
}

void printstr(char *val)
{
    while(*val)
    {
        putch(*val);
        val++;
    }
    return;
}

static char *__itoa(int num, char *buff, uint16_t base)
{
  static const char sym[] = "0123456789abcdef";

  char tmp[32];
  bool is_neg = false;
  if (num == 0)
  {
    strcpy(buff, "0");
    return buff;
  }
  else if (num < 0)
  {
    is_neg = true;
    strcpy(buff, "-");
    buff++;
    num = -num;
  }

  uint8_t i = 0;
  while (num != 0)
  {
    tmp[i] = sym[num % base];
    num /= base;
    i++;
  }

  for (int j = i - 1; j >= 0; --j)
    buff[i - 1 - j] = tmp[j];
  buff[i] = '\0';

  return is_neg ? (buff - 1) : buff;
}

static char *__ptoa(void *p, char *buff)
{
  static const char sym[] = "0123456789abcdef";

  uint32_t num = (uint32_t)p;
  char tmp[32];

  if (num == 0)
  {
    buff[0] = '0';
    buff[1] = '\0';
    return (buff - 1);
  }

  uint8_t i = 0;
  while (num != 0)
  {
    tmp[i] = sym[num % 16];
    num /= 16;
    i++;
  }

  for (int j = i - 1; j >= 0; --j)
    buff[i - 1 - j] = tmp[j];
  buff[i] = '\0';

  return (buff - 1);
}

int sprintint(int val,char *out)
{
    int i;
    if(val == 0)
        return 0;
    i = sprintint(val/10,out);
    *(out+i) = (val%10) + '0';
    return i+1;
}

void sprintstr(char *val, char *out)
{
    strcpy(out,val);
    return;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap,fmt);
    while(*fmt)
    {
        if(*fmt != '%')
        {
            putch(*fmt);
            fmt++;
        }
        else
        {
            fmt++;
            switch(*fmt)
            {
                case 'c':
                {
                    char val = va_arg(ap,int);
                    putch(val);
                    fmt++;
                    break;
                }
                case 'd':
                case 'x':
                {
                    int val = va_arg(ap,int);
                    char buff[32];
                    __itoa(val,buff,(*fmt=='x')?16:10);
                    for(int i=0;buff[i]!='\0';i++)
                        putch(buff[i]);
                    fmt++;
                    break;
                }
                case 's':
                {
                    char *val = va_arg(ap,char *);
                    printstr(val);
                    fmt++;
                    break;
                }
                case 'p':
                {
                    void *val = va_arg(ap,void *);
                    char buff[32];
                    __ptoa(val,buff);
                    for(int i=0;buff[i]!='\0';i++)
                        putch(buff[i]);
                    fmt++;
                    break;
                }
                default:
                {
                    putch(*fmt);
                    fmt++;
                }
            }
        }
    }
    va_end(ap);
    return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    int i=0;
    va_list ap;
    va_start(ap,fmt);
    while(*fmt)
    {
        if(*fmt != '%')
        {
            *out++=*fmt;
            fmt++;
            i++;
        }
        else
        {
            fmt++;
            switch(*fmt)
            {
                case 'c':
                {
                    char val = va_arg(ap,int);
                    *out++=val;
                    i++;
                    fmt++;
                    break;
                }
                case 'd':
                {
                    int val = va_arg(ap,int);
                    int j = sprintint(val,out);
                    out += j;
                    i+=j;
                    fmt++;
                    break;
                }
                case 's':
                {
                    char *val = va_arg(ap,char*);
                    sprintstr(val,out);
                    out += strlen(val);
                    i += strlen(val);
                    fmt++;
                    break;
                }
                default:
                {
                    *out++=*fmt++;
                    i++;
                }
            }
        }
    }
    *out='\0';
    va_end(ap);
    return i ;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    int i=0;
    va_list ap;
    va_start(ap,fmt);
    while(*fmt && i!=n)
    {
        if(*fmt != '%')
        {
            *out++=*fmt;
            fmt++;
            i++;
        }
        else
        {
            fmt++;
            switch(*fmt)
            {
                case 'c':
                {
                    char val = va_arg(ap,int);
                    *out++=val;
                    i++;
                    fmt++;
                    break;
                }
                case 'd':
                {
                    int val = va_arg(ap,int);
                    int j = sprintint(val,out);
                    out += j;
                    i+=j;
                    fmt++;
                    break;
                }
                case 's':
                {
                    char *val = va_arg(ap,char*);
                    sprintstr(val,out);
                    out += strlen(val);
                    i += strlen(val);
                    fmt++;
                    break;
                }
                default:
                {
                    *out++=*fmt++;
                    i++;
                }
            }
        }
    }
    *out='\0';
    va_end(ap);
    return i ;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
