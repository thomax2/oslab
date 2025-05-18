#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#if defined(CONFIG_ISA64)
typedef uint64_t word_t;
#else
typedef uint32_t word_t;
#endif

unsigned long m_pow_n(unsigned long m, unsigned long n)
{
    unsigned long i = 0, ret = 1;
    if (n < 0)
        return 0;
    for (i = 0; i < n; i++)
    {
        ret *= m;
    }
    return ret;
}

int printf(const char *fmt, ...)
{

    char buf[256];

    va_list args;
    va_start(args, fmt); // args point to the last know args in variable args
    int num = vsprintf(buf, fmt, args);
    va_end(args);

    for (int i = 0; i < num; ++i)
    {
        putch(buf[i]);
    }

    return num;
}

static char *__itoa(int num, char *buff, int base)
{
    static const char sym[] = "0123456789abcdef";
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

    int i = 1;
    while ((int)(num / (i * base)) != 0)
        i *= base;

    int j = 0;
    for (; i != 0; j++)
    {
        buff[j] = sym[num / i];
        num = num % i;
        i /= base;
    }
    buff[j] = '\0';
    return is_neg ? (buff - 1) : buff;
}

static char *__ptoa(void *p, char *buff)
{
    static const char sym[] = "0123456789abcdef";

    uintptr_t num = (uintptr_t)p;

    buff[0] = '0';
    buff++;
    buff[0] = 'x';
    buff++;
    uintptr_t i = 1;
    while ((uintptr_t)(num / (i * 16)) != 0)
        i *= 16;

    int j = 0;
    for (; i != 0; j++)
    {
        buff[j] = sym[num / i];
        num = num % i;
        i /= 16;
    }

    buff[j] = '\0';
    return buff - 2;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
    char *p;
    int pc = 0;
    int fill_symp = 0; // 0 space, 1 0, 2 - left, 3 + sign
                       // 4 #  special formal
    int char_length = 0;
    bool is_end = false;

    for (p = out; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *(p++) = *fmt;
            continue;
        }
        is_end = false;
        pc = 0;
        fill_symp = 0;
        char_length = 0;
        while (!is_end)
        {
            fmt++;
            switch (pc)
            {
            case 0:
                switch (*fmt)
                {
                case '0':
                    fill_symp = 1;
                    break;
                case '-':
                    fill_symp = 2;
                    break;
                case '+':
                    fill_symp = 3;
                    break;
                case '#':
                    fill_symp = 4;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    char_length = *fmt - '0';
                    break;
                }
                case 'd':
                case 'i':
                case 'x':
                {
                    char istr[20];
                    int val = va_arg(ap, int);
                    __itoa(val, istr, (*fmt == 'x') ? 16 : 10);
                    strcpy(p, istr);
                    p += strlen(istr);
                    is_end = true;
                    break;
                }
                case 'p':
                {
                    char istr[20];
                    void *val = va_arg(ap, void *);
                    __ptoa(val, istr);
                    strcpy(p, istr);
                    p += strlen(istr);
                    is_end = true;
                    break;
                }
                case 'c':
                {
                    int temp = va_arg(ap, int);
                    char val = (char)temp;
                    *(p++) = val;
                    is_end = true;
                    break;
                }
                case 's':
                {
                    const char *str = va_arg(ap, char *);
                    strcpy(p, str);
                    p += strlen(str);

                    is_end = true;
                    break;
                }
                default:
                    break;
                }
                break;
            case 1:
                if (fill_symp)
                {
                    switch (*fmt)
                    {
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    {
                        char_length = *fmt - '0';
                        break;
                    }
                    case 'd':
                    case 'i':
                    case 'x':
                    {
                        char istr[20];
                        int val = va_arg(ap, int);
                        __itoa(val, istr, (*fmt == 'x') ? 16 : 10);
                        if (fill_symp == 3)
                        {
                            if (val >= 0)
                                *(p++) = '+';
                            strcpy(p, istr);
                            p += strlen(istr);
                        }
                        else if (fill_symp == 4 && *fmt == 'x')
                        {
                            *(p++) = '0';
                            *(p++) = 'x';
                            strcpy(p, istr);
                            p += strlen(istr);
                        }
                        else
                        {
                            strcpy(p, istr);
                            p += strlen(istr);
                        }

                        is_end = true;
                        break;
                    }
                    case 'p':
                    {
                        char istr[20];
                        void *val = va_arg(ap, void *);
                        __ptoa(val, istr);
                        strcpy(p, istr);
                        p += strlen(istr);

                        is_end = true;
                        break;
                    }
                    case 's':
                    {
                        const char *str = va_arg(ap, char *);
                        strcpy(p, str);
                        p += strlen(str);

                        is_end = true;
                        break;
                    }
                    case 'c':
                    {
                        int temp = va_arg(ap, int);
                        char val = (char)temp;

                        *(p++) = val;

                        is_end = true;
                        break;
                    }
                    default:
                        break;
                    }
                }
                else if (char_length)
                {
                    switch (*fmt)
                    {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    {
                        char_length = char_length * 10 + *fmt - '0';
                        break;
                    }
                    case 'd':
                    case 'i':
                    case 'x':
                    {
                        char istr[40];
                        int val = va_arg(ap, int);
                        __itoa(val, istr, (*fmt == 'x') ? 16 : 10);
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                        strcpy(p, istr);
                        p += strlen(istr);

                        is_end = true;
                        break;
                    }
                    case 'p':
                    {
                        char istr[20];
                        void *val = va_arg(ap, void *);
                        __ptoa(val, istr);
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                        strcpy(p, istr);
                        p += strlen(istr);

                        is_end = true;
                        break;
                    }
                    case 's':
                    {
                        const char *str = va_arg(ap, char *);

                        for (int j = 0; j < char_length - (int)strlen(str); j++)
                            *(p++) = ' ';

                        strcpy(p, str);
                        p += strlen(str);
                        is_end = true;
                        break;
                    }
                    case 'c':
                    {
                        int temp = va_arg(ap, int);
                        char val = (char)temp;

                        for (int j = 0; j < char_length - 1; j++)
                            *(p++) = ' ';
                        *(p++) = val;
                        is_end = true;
                        break;
                    }
                    default:
                        break;
                    }
                }
                break;
            case 2:
                switch (*fmt)
                {
                case 'd':
                case 'i':
                case 'x':
                {
                    char istr[40];
                    int val = va_arg(ap, int);
                    __itoa(val, istr, (*fmt == 'x') ? 16 : 10);
                    if (fill_symp == 2)
                    {
                        strcpy(p, istr);
                        p += strlen(istr);
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                    }
                    else if (fill_symp == 1)
                    {
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = '0';
                        strcpy(p, istr);
                        p += strlen(istr);
                    }
                    else if (fill_symp == 3)
                    {
                        if (val >= 0)
                        {
                            for (int j = 0; j < char_length - (int)strlen(istr) - 1; j++)
                                *(p++) = ' ';
                            *(p++) = '+';
                        }
                        else
                            for (int j = 0; j < char_length - (int)strlen(istr) - 1; j++)
                                *(p++) = ' ';

                        strcpy(p, istr);
                        p += strlen(istr);
                    }
                    else if (fill_symp == 4 && *fmt == 'x')
                    {
                        for (int j = 0; j < char_length - (int)strlen(istr) - 2; j++)
                            *(p++) = ' ';
                        *(p++) = '0';
                        *(p++) = 'x';
                        strcpy(p, istr);
                        p += strlen(istr);
                    }
                    else
                    {
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                        strcpy(p, istr);
                        p += strlen(istr);
                    }

                    is_end = true;
                    break;
                }
                case 'p':
                {
                    char istr[20];
                    void *val = va_arg(ap, void *);
                    __ptoa(val, istr);
                    if (fill_symp == 2)
                    {
                        strcpy(p, istr);
                        p += strlen(istr);
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                    }
                    else
                    {
                        for (int j = 0; j < char_length - (int)strlen(istr); j++)
                            *(p++) = ' ';
                        strcpy(p, istr);
                        p += strlen(istr);
                    }

                    is_end = true;
                    break;
                }
                case 's':
                {
                    const char *str = va_arg(ap, char *);

                    if (fill_symp == 2)
                    {
                        strcpy(p, str);
                        p += strlen(str);
                        for (int j = 0; j < char_length - (int)strlen(str); j++)
                            *(p++) = ' ';
                    }
                    else
                    {
                        for (int j = 0; j < char_length - (int)strlen(str); j++)
                            *(p++) = ' ';
                        strcpy(p, str);
                        p += strlen(str);
                    }
                    is_end = true;
                    break;
                }
                case 'c':
                {
                    int temp = va_arg(ap, int);
                    char val = (char)temp;

                    if (fill_symp == 2)
                    {
                        *(p++) = val;
                        for (int j = 0; j < char_length - 1; j++)
                            *(p++) = ' ';
                    }
                    else if (fill_symp == 1)
                    {
                        for (int j = 0; j < char_length - 1; j++)
                            *(p++) = '0';
                        *(p++) = val;
                    }
                    else
                    {
                        for (int j = 0; j < char_length - 1; j++)
                            *(p++) = ' ';
                        *(p++) = val;
                    }
                    is_end = true;
                    break;
                }
                default:
                    break;
                }
                break;
            default:
                break;
            }
            pc++;
        }
    }
    *p = '\0';

    return strlen(out);
}

int sprintf(char *out, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int num = vsprintf(out, fmt, ap);
    va_end(ap);
  
    return num;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
    va_list pArgs;
    va_start(pArgs, fmt);
    return vsnprintf(out, n, fmt, pArgs);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
    if (fmt == NULL)
        return -1;
    int ret_num = 0;
    char *pStr = (char *)fmt; // 指向str
    int ArgIntVal = 0;        // 接收整型
    // unsigned long ArgHexVal = 0;// 接十六进制
    char *ArgStrVal = NULL; // 接收字符型
    // double ArgFloVal = 0.0; // 接受浮点型
    unsigned long val_seg = 0; // 数据切分
    // unsigned long val_temp = 0;  // 临时保存数据
    int cnt = 0; // 数据长度计数
    char *str_out = out;
    for (; *pStr != '\0'; pStr++)
    {
        switch (*pStr)
        {
        case '%':
            pStr++;
            switch (*pStr)
            {
            case '%':
                if (ret_num + 1 > n)
                    return ret_num;
                *str_out++ = *pStr;
                ret_num++;
                break;
            case 'd':
                ArgIntVal = va_arg(ap, int);
                if (ArgIntVal < 0) // 如果为负数打印，负号
                {
                    ArgIntVal = -ArgIntVal; // 取相反数
                    if (ret_num + 1 > n)
                        return ret_num;
                    *str_out++ = '-';
                    ret_num++;
                }
                val_seg = ArgIntVal; // 赋值给 val_seg处理数据
                // 计算ArgIntVal长度
                if (ArgIntVal)
                {
                    while (val_seg)
                    {
                        cnt++;
                        val_seg /= 10;
                    }
                }
                else
                    cnt = 1; // 数字0的长度为1
                // 将整数转为单个字符打印
                while (cnt)
                {
                    val_seg = ArgIntVal / m_pow_n(10, cnt - 1);
                    ArgIntVal %= m_pow_n(10, cnt - 1);
                    if (ret_num + 1 > n)
                        return ret_num;
                    *str_out++ = (char)val_seg + '0';
                    ret_num++;
                    cnt--;
                }
                break;
            case 'c':
                ArgIntVal = va_arg(ap, int);
                if (ret_num + 1 > n)
                    return ret_num;
                *str_out++ = (char)ArgIntVal;
                ret_num++;
                break;
            case 's':
                ArgStrVal = va_arg(ap, char *);
                for (; *ArgStrVal != '\0'; ArgStrVal++)
                {
                    if (ret_num + 1 > n)
                        return ret_num;
                    *str_out++ = *ArgStrVal;
                    ret_num++;
                }
                break;
            default:
                break;
            }
            break;
        default:
            if (ret_num + 1 > n)
                return ret_num;
            *str_out++ = *pStr;
            ret_num++;
            break;
        }
    }
    *str_out = '\0';
    assert(ret_num = strlen(out));
    assert(ret_num <= n);
    return ret_num;
}

#endif
