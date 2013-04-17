/* wget http://fctx.wildbearsoftware.com/static/fctx/download/1.5.0/fct.h */ 
/* gcc ../hash-je/hash.c ../hash-je/stash.c cssdom.c test.c */

#include "fct.h"

#include "cssdom.h"

FCT_BGN() {
    FCT_QTEST_BGN(strstartcmp_test_1) {
        fct_chk_eq_int(strstartcmp("en", "en"), 0);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strstartcmp_test_2) {
        fct_chk_eq_int(strstartcmp("en-GB", "en"), 0);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strstartcmp_test_3) {
        fct_chk_eq_int(strstartcmp("en-GB", "eng"), 1);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strstartcmp_test_4) {
        fct_chk_eq_int(strstartcmp("en-GB", "e"), 1);
    } FCT_QTEST_END();
	FCT_QTEST_BGN(strstartcmp_test_5) {
        fct_chk_eq_int(strstartcmp("en-GB", ""), 1);
    } FCT_QTEST_END();    
	FCT_QTEST_BGN(strstartcmp_test_6) {
        fct_chk_eq_int(strstartcmp("", "en"), 1);
    } FCT_QTEST_END();    
	FCT_QTEST_BGN(strstartcmp_test_7) {
        fct_chk_eq_int(strstartcmp("english", "eng"), 1);
    } FCT_QTEST_END();
    
   	FCT_QTEST_BGN(strspacecmp_test_1) {
        fct_chk_eq_int(strspacecmp("class1 class2 class3", "class1"), 0);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strspacecmp_test_2) {
        fct_chk_eq_int(strspacecmp("class1 class2 class3", "class2"), 0);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strspacecmp_test_3) {
        fct_chk_eq_int(strspacecmp("class1 class2 class3", "class3"), 0);
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strspacecmp_test_4) {
        fct_chk_eq_int(strspacecmp("class1 class2 class3", "class4"), 1);
    } FCT_QTEST_END();
	FCT_QTEST_BGN(strspacecmp_test_5) {
        fct_chk_eq_int(strspacecmp("class1 class2 class3", "class"), 1);
    } FCT_QTEST_END();    
    
    FCT_QTEST_BGN(strtrim_test_1) {
    	char buf[80] = { "no-space" };
        fct_chk_eq_str(strtrim(buf), "no-space");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_2) {
    	char buf[80] = { " aaa" };
        fct_chk_eq_str(strtrim(buf), "aaa");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_3) {
    	char buf[80] = { "bbb " };
        fct_chk_eq_str(strtrim(buf), "bbb");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_4) {
    	char buf[80] = { " abc " };
        fct_chk_eq_str(strtrim(buf), "abc");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_5) {
    	char buf[80] = { "" };
        fct_chk_eq_str(strtrim(buf), "");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_6) {
    	char buf[80] = { "                      " };
        fct_chk_eq_str(strtrim(buf), "");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_7) {
    	char buf[80] = { "         !             " };
        fct_chk_eq_str(strtrim(buf), "!");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_8) {
    	char buf[80] = { "      \n   !             " };
        fct_chk_eq_str(strtrim(buf), "!");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_9) {
    	char buf[80] = { "      \n   !             " };
        fct_chk_eq_str(strtrim(buf), "!");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_10) {
    	char buf[80] = { "no trim needed" };
        fct_chk_eq_str(strtrim(buf), "no trim needed");
    } FCT_QTEST_END();
    FCT_QTEST_BGN(strtrim_test_11) {
    	char buf[80] = { "no_trim_needed" };
        fct_chk_eq_str(strtrim(buf), "no_trim_needed");
    } FCT_QTEST_END();
	FCT_QTEST_BGN(strtrim_test_12) {
    	char buf[80] = { "no-trim-needed" };
        fct_chk_eq_str(strtrim(buf), "no-trim-needed");
    } FCT_QTEST_END();
} FCT_END();
