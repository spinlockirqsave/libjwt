/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>

#include <jwt.h>

static void *test_malloc(size_t size)
{
	return malloc(size);
}

static void test_free(void *ptr)
{
	free(ptr);
}

static void *test_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

static int test_set_alloc(void)
{
	return jwt_set_alloc(test_malloc, test_realloc, test_free);
}

START_TEST(test_alloc_funcs)
{
	jwt_malloc_t m = NULL;
	jwt_realloc_t r = NULL;
	jwt_free_t f = NULL;
	int ret;

	jwt_get_alloc(&m, &r, &f);
	ck_assert(m == NULL);
	ck_assert(r == NULL);
	ck_assert(f == NULL);

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	jwt_get_alloc(&m, &r, &f);
	ck_assert(m == test_malloc);
	ck_assert(r == test_realloc);
	ck_assert(f == test_free);
}
END_TEST

START_TEST(test_jwt_dump_fp)
{
	FILE *out;
	jwt_t *jwt = NULL;
	int ret = 0;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert(jwt != NULL);

	ret = jwt_add_grant(jwt, "iss", "files.cyphre.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

#ifdef _WIN32
	out = fopen("nul", "w");
	ck_assert(out != NULL);
#else
	out = fopen("/dev/null", "w");
	ck_assert(out != NULL);
#endif

	ret = jwt_dump_fp(jwt, out, 1);
	ck_assert_int_eq(ret, 0);

	ret = jwt_dump_fp(jwt, out, 0);
	ck_assert_int_eq(ret, 0);

	fclose(out);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_dump_str)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert(jwt != NULL);

	ret = jwt_add_grant(jwt, "iss", "files.cyphre.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	out = jwt_dump_str(jwt, 1);
	ck_assert(out != NULL);

	jwt_free_str(out);

	out = jwt_dump_str(jwt, 0);
	ck_assert(out != NULL);

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_dump_str_alg)
{
	jwt_t *jwt = NULL;
	const char key[] = "My Passphrase";
	int ret = 0;
	char *out;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert(jwt != NULL);

	ret = jwt_add_grant(jwt, "iss", "files.cyphre.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	ret = jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)key,
			  strlen(key));
	ck_assert_int_eq(ret, 0);

	out = jwt_dump_str(jwt, 1);
	ck_assert(out != NULL);

	jwt_free_str(out);

	out = jwt_dump_str(jwt, 0);
	ck_assert(out != NULL);

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

static Suite *libjwt_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LibJWT Dump");

	tc_core = tcase_create("jwt_dump");

	tcase_add_test(tc_core, test_alloc_funcs);
	tcase_add_test(tc_core, test_jwt_dump_fp);
	tcase_add_test(tc_core, test_jwt_dump_str);
	tcase_add_test(tc_core, test_jwt_dump_str_alg);

	tcase_set_timeout(tc_core, 30);

	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = libjwt_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
