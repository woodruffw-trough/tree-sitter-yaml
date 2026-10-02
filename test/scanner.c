#include "../src/scanner.c"

static void test_serialization_bounds(void) {
    Scanner scanner = {0};
    Scanner restored = {0};
    char *buffer = malloc(TREE_SITTER_SERIALIZATION_BUFFER_SIZE);
    assert(buffer != NULL);
    deserialize(&scanner, NULL, 0);

    unsigned header_size = serialize(&scanner, buffer);
    unsigned entry_size = 2 * sizeof(int16_t);
    unsigned capacity = (TREE_SITTER_SERIALIZATION_BUFFER_SIZE - header_size) / entry_size;

    for (unsigned count = 1; count <= capacity + 2; count++) {
        push_ind(&scanner, IND_MAP, (int16_t)count);
        unsigned length = serialize(&scanner, buffer);
        unsigned saved = count < capacity ? count : capacity;
        assert(length <= TREE_SITTER_SERIALIZATION_BUFFER_SIZE);
        assert(length == header_size + saved * entry_size);

        deserialize(&restored, buffer, length);
        assert(restored.ind_typ_stk.size == saved + 1);
        assert(restored.ind_len_stk.size == saved + 1);
        for (unsigned i = 1; i <= saved; i++) {
            assert(restored.ind_typ_stk.contents[i] == IND_MAP);
            assert(restored.ind_len_stk.contents[i] == (int16_t)i);
        }
    }

    array_delete(&scanner.ind_typ_stk);
    array_delete(&scanner.ind_len_stk);
    array_delete(&restored.ind_typ_stk);
    array_delete(&restored.ind_len_stk);
    free(buffer);
}

int main(void) {
    test_serialization_bounds();
    return 0;
}
