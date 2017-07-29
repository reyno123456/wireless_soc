static int s_errno = 0;

int * __errno()
{
    return &s_errno;
}
