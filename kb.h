#ifndef KBTREE_WRAPPER_H__
#define KBTREE_WRAPPER_H__
#include "klib/kbtree.h"
#include <stdexcept>
#include <string>

#if 0
	typedef struct {							\
		kbnode_t *root;							\
		int	off_key, off_ptr, ilen, elen;		\
		int	n, t;								\
		int	n_keys, n_nodes;					\
	} kbtree_##name##_t;
#define MAX_DEPTH 64

typedef struct {
	int32_t is_internal:1, n:31;
} kbnode_t;

typedef struct {
	kbnode_t *x;
	int i;
} kbpos_t;

typedef struct {
	kbpos_t stack[MAX_DEPTH], *p;
} kbitr_t;
#endif

namespace kb {

struct DefaultCmp {
    template<typename T>
    int operator()(const T &a, const T &b) const {
        return (a < b) ? -1: b < a ? 1: 0;
    }
};

template<typename KeyType, typename Cmp=DefaultCmp, size_t MAX_DEPTH_PARAM=64>
class KBTree {
public:
    using key_t = KeyType;
    static constexpr size_t MAX_DEPTH = MAX_DEPTH_PARAM;

    struct node_t {int32_t is_internal:1, n:31;};
    struct pos_t  {node_t *x; int i;};
    struct iter_t {pos_t stack[MAX_DEPTH], *p;};

    int t, n;
    int off_key, off_ptr, ilen, elen;
    node_t *root;
    int n_keys, n_nodes;
    KBTree(size_t size=KB_DEFAULT_SIZE):
        t(((size - 4 - sizeof(void *)) / (sizeof(void *) + sizeof(key_t)) + 1) >>1), n((t<<1) - 1),
        off_ptr(4 + n * sizeof(key_t)), ilen((4 + sizeof(void *) + n * (sizeof(void *) + sizeof(key_t)) + 3) >> 2 << 2),
        root(t >= 2 ? static_cast<node_t *>(std::calloc(1, ilen)): nullptr),
        off_key(0),
        elen((off_ptr + 3) >> 2 << 2), n_keys(0), n_nodes(1)
    {
        if(!root) throw std::invalid_argument(std::string("t must be >= 2. t: ") + std::to_string(t));
    };
    node_t **ptr(node_t *x) {
        return reinterpret_cast<node_t **>(reinterpret_cast<char *>(this) + off_ptr);
    }
    const node_t **ptr(node_t *x) const {
        return reinterpret_cast<const node_t **>(reinterpret_cast<const char *>(this) + off_ptr);
    }
    ~KBTree() {
        int i, max = 8;
        node_t *x, **top, **stack = 0;
        top = stack = static_cast<node_t **>(std::calloc(max, sizeof(node_t *)));
        *top++ = root;
        while(top != stack) {
            x = *--top;
            if(x->is_internal == 0) {std::free(x); continue;}
            for(i = 0; i <= n; ++i) {
                if (ptr(x)[i]) {
                    if(top - stack == max) {
                        max <<= 1;
                        stack = static_cast<node_t **>(std::realloc(stack, max * sizeof(node_t *)));
                    }
                    *top++ = ptr(x)[i];
                }
                std::free(x);
            }
            std::free(stack);
        }
    }
    int cmp(const KeyType &a, const KeyType &b) const {return Cmp()(a, b);}
    static key_t *key(node_t *node) {
        return reinterpret_cast<key_t *>(reinterpret_cast<char *>(node) + 4);
    }
    static const key_t *key(const node_t *node) {
        return reinterpret_cast<const key_t *>(reinterpret_cast<const char *>(node) + 4);
    }
    static int get_aux1(const node_t * __restrict x, const key_t * __restrict k, int *r) {
		int tr, *rr, begin = 0, end = x->n;
		if (x->n == 0) return -1;
        rr = r ? r: &tr;
        while(begin < end) {
            int mid = (begin + end) >> 1;
            if(Cmp()(key(x)[mid], *k) < 0) begin = mid + 1;
            else end = mid;
        }
        if(begin == x->n) { *rr = 1; return x->n - 1;}
        begin -= (*rr = Cmp()(*k, key(x)[begin])) < 0;
        return begin;
    }
    key_t *get(const key_t * __restrict k) {
		int i, r = 0;
		node_t *x = root;
		while (x) {
			i = get_aux1(x, k, &r);
            if(i >= 0 && r == 0) return &key(x)[i];
			if (x->is_internal == 0) return nullptr;
			x = ptr(x)[i + 1];
		}
        return nullptr;
    }
    const key_t *get(const key_t * __restrict k) const {
		int i, r = 0;
		node_t *x = root;
		while (x) {
			i = get_aux1(x, k, &r);
            if(i >= 0 && r == 0) return &key(x)[i];
			if (x->is_internal == 0) return nullptr;
			x = ptr(x)[i + 1];
		}
        return nullptr;
    }
    key_t *get(key_t k) {return get(&k);}
    const key_t *get(key_t k) const {return get(&k);}
    auto size() const {return n_keys;}
    // TODO: Interval, Split, Put, Iterator interface
#if 0
#endif
};


} // namespace kb

#endif
