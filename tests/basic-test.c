#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "dict.h"

#define SEED 0xdeadbeef

char *keys[] = {
    "YXQXXDIP",
    "BSWANXGU",
    "QDBNUHYG",
    "CLOOXLBD",
    "FLLFIYSA",
    "KHPXKQAI",
    "GSGLZNNK",
    "NDMKXZIO",
    "YKKICAEJ",
    "KHTPBDSC",
    "HCIBQWJW",
    "USFYLXVX",
    "BKHPMJVI",
    "VHEGHBDN",
    "BNUGVTBW",
    "INZPKQUS",
    "SGJTBWER",
    "DZIRUWHW",
    "OFGPKTVA",
    "PZXGMHZV",
    "HOANJTBY",
    "TNKTWWZX",
    "OYVQCGYL",
    "JZOKARIC",
    "NYNDFVKO",
    "QAYOOOJK",
    "RMIVHDHT",
    "OXUOIQBO",
    "QEKEIEBS",
    "TFDZHLMT",
    "JUAAPPLY",
    "FHSHETTM",
    "GMPIQJCM",
    "SREBDAGT",
    "YSCQGVHX",
    "BHOQBTMV",
    "CEVWZSUT",
    "QAPZETFP",
    "KUZCULKT",
    "AUQVCYCI",
    "ONIEWEBM",
    "AUKANCQU",
    "SDBUYDYG",
    "VBWMYFAC",
    "OUXCAZYD",
    "LLGUNMOC",
    "UWHYIIZG",
    "YXTXHPYI",
    "ZWKUWKLU",
    "ONJXWJKK",
    "QXNDTAVM",
    "KMLTNXTW",
    "THTCZCFD",
    "UZWIFAXF",
    "PKPZYLQT",
    "XPGPGLDR",
    "OEFEZTAV",
    "COACHKOD",
    "NJZMYBVC",
    "TIWGXQTR",
    "GHVKQQZP",
    "XUFZBJJX",
    "NBDDGPGY",
    "KGDVXTCR"
};

char *values[] = {
    "NBAYBPOZ",
    "ZLIBYSFZ",
    "YFYQMXEO",
    "VMMHJVOR",
    "DKAYLAUF",
    "CXIVQVJX",
    "OOKLJSKC",
    "VPSEUMMN",
    "LAGDIPVK",
    "IOTYHKQK",
    "KOGTNLQR",
    "DLAASCHL",
    "SYSLSSXK",
    "UJGCKDHT",
    "FRUBYAEB",
    "XQNHOPGC",
    "CFOMYZJB",
    "UKWGNUBA",
    "BTRNDDDX",
    "CDVYNEWY",
    "KVDCSCUT",
    "JAMCCJIA",
    "ZZSXQKKJ",
    "TUBWRGQH",
    "IREQWPHF",
    "TUUTFPWU",
    "CYHMWIIW",
    "AHWPYJZU",
    "GHEBHGOA",
    "KALBVGKK",
    "YYLSFQRD",
    "GPHEMSMU",
    "BJWKZDJO",
    "MTUQLFAF",
    "JDBFKYTP",
    "TJOKZZBO",
    "SMXUJZHO",
    "XNPYKLNP",
    "XHBEGSMD",
    "ZPKKEEUT",
    "NJSUDWQO",
    "BYCDKVLR",
    "QZFDOAWL",
    "EOEUYQSN",
    "OMHGUFWB",
    "CYGEXIVZ",
    "IKLPTDAP",
    "WFGYXARJ",
    "YHIKHJSV",
    "RUVQMOZU",
    "ANVKZOEQ",
    "HTCZZQQO",
    "NBJKVIFM",
    "EXMYVTSZ",
    "ERJXJXLG",
    "NDUMCCHE",
    "ZCHYISTB",
    "YMDXVEOK",
    "KJVNWEHR",
    "IXMOLXJZ",
    "RVDUAJYW",
    "KOYLHHME",
    "SCIBVKXW",
    "IYILYYJJ"
};

size_t items = sizeof(keys) / sizeof(keys[0]);

void test_values_correct(struct dict *d)
{
    size_t i;
    struct dict_node *n;
    
    for (i = 0; i < items; i++) {
        // Test dict_contains()
        assert(dict_contains(d, keys[i]) == 1);
        
        // Test dict_get()
        assert((n = dict_get(d, keys[i])) != NULL);
        
        // Test that value is correct
        assert(strcmp(n->value, values[i]) == 0);
    }
}

int main(void)
{
    size_t i;
    struct dict *d;
    
    d = dict_new(SEED, 512, NULL, NULL);
    for (i = 0; i < items; i++) {
        // Test dict_set()
        assert(dict_set(d, keys[i], values[i]) == 1);
    }
    
    // Test that values are correct
    test_values_correct(d);
    
    // Test resize... Overfills dict, forcing hash collisions
    assert(dict_resize(d, 16) == 1);
    
    // Test that values are still correct
    test_values_correct(d);
    
    // Test deleting first 16 items
    for (i = 0; i < 16; i++) {
        assert(dict_del(d, keys[i]) == 1);
    }
    
    // Make sure nothing got garbaged
    for (i = 0; i < items; i++) {
        if (i < 16) {
            // Make sure delete items are gone
            assert(dict_contains(d, keys[i]) == 0);
        } else {
            // Make sure non-deleted items are still there
            assert(dict_contains(d, keys[i]) == 1);
        }
    }
    
    // Test dict_clear()
    dict_clear(d);
    
    // Make sure object is empty
    for (i = 0; i < items; i++) {
        assert(dict_contains(d, keys[i]) == 0);
    }
    
    // More precise "empty" test, that pokes into data structure
    for (i = 0; i < d->capacity; i++) {
        assert(d->table[i].hash == 0);
        assert(d->table[i].key == NULL);
        assert(d->table[i].value == NULL);
        assert(d->table[i].next == NULL);
    }
    
    dict_delete(d);
    exit(0);
}
