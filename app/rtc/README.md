# RTC 应用开发

[韦东山教程](https://blog.51cto.com/weidongshan/4795719#104_RTC_364)

tm 结构的定义如下：

```c
struct tm
{
   int tm_sec;         /* 秒，范围从 0 到 59 */
   int tm_min;         /* 分，范围从 0 到 59 */
   int tm_hour;        /* 小时，范围从 0 到 23  */
   int tm_mday;        /* 一月中的第几天，范围从 1 到 31 */
   int tm_mon;         /* 月，范围从 0 到 11(注意) */
   int tm_year;        /* 自 1900 年起的年数 */
   int tm_wday;        /* 一周中的第几天，范围从 0 到 6 */
   int tm_yday;        /* 一年中的第几天，范围从 0 到 365 */
   int tm_isdst;       /* 夏令时 */
};
```
