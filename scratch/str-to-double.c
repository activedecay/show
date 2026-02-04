#include <stddef.h>
#include <limits.h>
#define CSAPP_IMPLEMENTATION
#include "../src/lib/csapp.h"

int main(int argc, char *argv[]) {
  char *check;
  char *maybe_number;
  if (argc > 1)
    maybe_number = argv[1];
  else {
    app_error("no arg given");
    return -1;
  }
  long double d = /*safe_*/strtold(maybe_number, &check);
  int ret = 0;
  if (maybe_number == check) {
    app_error("no number"); /* No number was detected in string */
    ret = 1;
  } else if ((d == HUGE_VALL || d == -HUGE_VALL) && errno == ERANGE) {
    app_error("overflow"); /* numeric overflow in in string */
    ret = 2;
  } else if (d == 0 && errno == ERANGE) {
    // zero is returned and ERANGE is stored in errno
    app_error("underflow"); /* numeric underflow in in string */
    ret = 3;
  }
  ptrdiff_t diff = check - maybe_number;
  size_t len = strlen(maybe_number);
  printf("%Lg len(%lu) diff(%td)\n", d, len, diff);
  return ret;
}

int
man_example_main(int argc, char *argv[])
{
  int base;
  char *endptr, *str;
  long val;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s str [base]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  str = argv[1];
  base = (argc > 2) ? atoi(argv[2]) : 10;

  errno = 0;    /* To distinguish success/failure after call */
  val = strtol(str, &endptr, base);

  /* Check for various possible errors */

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      || (errno != 0 && val == 0)) {
    perror("strtol");
    exit(EXIT_FAILURE);
  }

  if (endptr == str) {
    fprintf(stderr, "No digits were found\n");
    exit(EXIT_FAILURE);
  }

  /* If we got here, strtol() successfully parsed a number */

  printf("strtol() returned %ld\n", val);

  if (*endptr != '\0')        /* Not necessarily an error... */
    printf("Further characters after number: %s\n", endptr);

  exit(EXIT_SUCCESS);
}
