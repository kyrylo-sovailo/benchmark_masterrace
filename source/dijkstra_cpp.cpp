#include <ctime>
#if defined(CMP_GPP) || defined(CMP_LLVMPP)
    #include <iostream>
    #include <fstream>
    #include <limits>
    #include <sstream>
    #include <vector>
    #define restrict
#endif
#include <string.h>

typedef struct
{
    unsigned int destination;
    float distance;
} Connection;

typedef struct
{
    unsigned int source;
    unsigned int destination;
} Benchmark;

typedef struct
{
    unsigned int id;
    unsigned int int_distance;
    float distance;
} Candidate;

#if defined (VEC_CPP)

typedef std::vector<Connection> ConnectionVector;
typedef std::vector<ConnectionVector> ConnectionVectorVector;
typedef std::vector<Benchmark> BenchmarkVector;
typedef std::vector<Candidate> CandidateVector;
typedef std::vector<unsigned int> UintVector;

#define VECTOR_INITIALIZE(VECTOR)
#define VECTOR_FINALIZE(VECTOR)
#define VECTOR_ACCESS(VECTOR, INDEX) (VECTOR)[INDEX]
#define VECTOR_RAW(VECTOR) (VECTOR).data()
#define VECTOR_EMPTY(VECTOR) ((VECTOR).empty())
#define VECTOR_LENGTH(VECTOR) ((VECTOR).size())
#define VECTOR_RESET(VECTOR) ((VECTOR).clear())
#define VECTOR_POP(VECTOR) ((VECTOR).pop_back())

#define VECTOR_UNPROTECTED_PUSH(TYPE, VECTOR, ELEMENT) ((VECTOR).push_back((ELEMENT)))
#define VECTOR_PUSH(TYPE, VECTOR, ELEMENT) ((VECTOR).push_back((ELEMENT)))
#define VECTOR_GROW(TYPE, VECTOR, SIZE) ((VECTOR).resize(SIZE))
#define VECTOR_TIGHT_GROW(TYPE, VECTOR, SIZE) ((VECTOR).resize(SIZE))
#define VECTOR_TIGHT_RESERVE(TYPE, VECTOR, SIZE) ((VECTOR).reserve(SIZE))

#else //#if defined (VEC_CPP)

#if defined(ALC_NEW)
#define ALLOCATE(POINTER, TYPE, NEW_NUMBER) POINTER = new TYPE[NEW_NUMBER]
#define REALLOCATE(POINTER, TYPE, OLD_NUMBER, NEW_NUMBER) { TYPE *old = POINTER; POINTER = new TYPE[NEW_NUMBER]; for (size_t i = 0; i < OLD_NUMBER; i++) POINTER[i] = old[i]; delete[] old; }
#define DEALLOCATE(POINTER) delete[] POINTER
#elif defined(ALC_NEW_NO_DELETE)
#define ALLOCATE(POINTER, TYPE, NEW_NUMBER) POINTER = new TYPE[NEW_NUMBER]
#define REALLOCATE(POINTER, TYPE, OLD_NUMBER, NEW_NUMBER) { TYPE *old = POINTER; POINTER = new TYPE[NEW_NUMBER]; for (size_t i = 0; i < OLD_NUMBER; i++) POINTER[i] = old[i]; }
#define DEALLOCATE(POINTER)
#elif defined(ALC_MALLOC)
#define ALLOCATE(POINTER, TYPE, NEW_NUMBER) POINTER = (TYPE*)malloc(NEW_NUMBER * sizeof(TYPE))
#define REALLOCATE(POINTER, TYPE, OLD_NUMBER, NEW_NUMBER) { TYPE *old = POINTER; POINTER = (TYPE*)malloc(NEW_NUMBER * sizeof(TYPE)); for (size_t i = 0; i < OLD_NUMBER; i++) POINTER[i] = old[i]; free(old); }
#define DEALLOCATE(POINTER) free(POINTER)
#elif defined(ALC_REALLOC)
#define ALLOCATE(POINTER, TYPE, NEW_NUMBER) POINTER = (TYPE*)malloc(NEW_NUMBER * sizeof(TYPE))
#define REALLOCATE(POINTER, TYPE, OLD_NUMBER, NEW_NUMBER) POINTER = (TYPE*)realloc(POINTER, NEW_NUMBER * sizeof(TYPE))
#define DEALLOCATE(POINTER) free(POINTER)
#elif defined(ALC_MALLOC_NO_FREE)
#define ALLOCATE(POINTER, TYPE, NEW_NUMBER) POINTER = (TYPE*)malloc(NEW_NUMBER * sizeof(TYPE))
#define REALLOCATE(POINTER, TYPE, OLD_NUMBER, NEW_NUMBER) { TYPE *old = POINTER; POINTER = (TYPE*)malloc(NEW_NUMBER * sizeof(TYPE)); for (size_t i = 0; i < OLD_NUMBER; i++) POINTER[i] = old[i]; }
#define DEALLOCATE(POINTER)
#elif defined(ALC_MMAP)
//TODO
#elif defined(ALC_STATIC)
//TODO
#endif

#if defined(VEC_POINTER)
#define VECTOR_DECLARE(TYPE, NAME) typedef struct { TYPE *p; TYPE *end; TYPE *end_capacity; } NAME ## Vector;
#elif defined(VEC_SIZE_T)
#define VECTOR_DECLARE(TYPE, NAME) typedef struct { TYPE *p; size_t length; size_t capacity; } NAME ## Vector;
#elif defined(VEC_UINT)
#define VECTOR_DECLARE(TYPE, NAME) typedef struct { TYPE *p; unsigned int length; unsigned int capacity; } NAME ## Vector;
#elif defined(VEC_POINTER_NO_CAPACITY)
#define VECTOR_DECLARE(TYPE, NAME) typedef struct { TYPE *p; TYPE *end; } NAME ## Vector;
#elif defined(VEC_SIZE_T_NO_CAPACITY)
#define VECTOR_DECLARE(TYPE, NAME) typedef struct { TYPE *p; size_t length; } NAME ## Vector;
#elif defined(VEC_UINT_OFFSET)
#endif

VECTOR_DECLARE(Connection, Connection);
VECTOR_DECLARE(ConnectionVector, ConnectionVector);
VECTOR_DECLARE(Benchmark, Benchmark);
VECTOR_DECLARE(Candidate, Candidate);
VECTOR_DECLARE(unsigned int, Uint);

#if defined(VEC_POINTER)

#define VECTOR_INITIALIZE(VECTOR) memset(&(VECTOR), 0, sizeof(VECTOR))
#define VECTOR_FINALIZE(VECTOR) { if ((VECTOR).p != NULL) { DEALLOCATE((VECTOR).p); } }
#define VECTOR_ACCESS(VECTOR, INDEX) (VECTOR).p[INDEX]
#define VECTOR_RAW(VECTOR) (VECTOR).p
#define VECTOR_EMPTY(VECTOR) ((VECTOR).end == (VECTOR).p)
#define VECTOR_LENGTH(VECTOR) ((size_t)((VECTOR).end - (VECTOR).p))
#define VECTOR_RESET(VECTOR) ((VECTOR).end = (VECTOR).p)
#define VECTOR_POP(VECTOR) (VECTOR).end--

#define VECTOR_UNPROTECTED_PUSH(TYPE, VECTOR, ELEMENT) (*((VECTOR).end++) = (ELEMENT))
#define VECTOR_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_RESERVE(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_reserve(&(VECTOR), SIZE)

void Connection_vector_push(ConnectionVector *connections, Connection connection)
{
    if (connections->end == connections->end_capacity)
    {
        const size_t old_capacity = connections->end_capacity - connections->p;
        const size_t new_capacity = (old_capacity == 0) ? 1 : (old_capacity << 1);
        REALLOCATE(connections->p, Connection, old_capacity, new_capacity);
        connections->end = connections->p + old_capacity;
        connections->end_capacity = connections->p + new_capacity;
    }
    *connections->end = connection;
    connections->end++;
}

void Benchmark_vector_push(BenchmarkVector *benchmarks, Benchmark benchmark)
{
    if (benchmarks->end == benchmarks->end_capacity)
    {
        const size_t old_capacity = benchmarks->end_capacity - benchmarks->p;
        const size_t new_capacity = (old_capacity == 0) ? 1 : (old_capacity << 1);
        REALLOCATE(benchmarks->p, Benchmark, old_capacity, new_capacity);
        benchmarks->end = benchmarks->p + old_capacity;
        benchmarks->end_capacity = benchmarks->p + new_capacity;
        
    }
    *benchmarks->end = benchmark;
    benchmarks->end++;
}

void ConnectionVector_vector_grow(ConnectionVectorVector *connections, size_t new_size)
{
    ConnectionVector *temp_new_end = connections->p + new_size;
    if (temp_new_end <= connections->end) return;
    if (temp_new_end <= connections->end_capacity)
    {
        connections->end = temp_new_end;
    }
    else
    {
        const size_t old_size = connections->end - connections->p;
        const size_t old_capacity = connections->end_capacity - connections->p;
        size_t new_capacity = (old_capacity == 0) ? 1 : (old_capacity << 1);
        while (new_size > new_capacity) new_capacity <<= 1;
        REALLOCATE(connections->p, ConnectionVector, old_size, new_capacity);
        connections->end = connections->p + new_size;
        connections->end_capacity = connections->p + new_capacity;
    }
}

void Uint_vector_tight_grow(UintVector *vector, size_t new_size)
{
    unsigned int *temp_new_end = vector->p + new_size;
    if (temp_new_end <= vector->end) return;
    if (temp_new_end <= vector->end_capacity)
    {
        vector->end = temp_new_end;
    }
    else
    {
        const size_t old_size = vector->end - vector->p;
        REALLOCATE(vector->p, unsigned int, old_size, new_size);
        vector->end = vector->p + new_size;
        vector->end_capacity = vector->p + new_size;
    }
}

void Candidate_vector_tight_reserve(CandidateVector *candidates, size_t new_capacity)
{
    Candidate *temp_new_capacity_end = candidates->p + new_capacity;
    if (temp_new_capacity_end <= candidates->end_capacity) {}
    else
    {
        const size_t old_size = candidates->end - candidates->p;
        REALLOCATE(candidates->p, Candidate, old_size, new_capacity);
        candidates->end = candidates->p + old_size;
        candidates->end_capacity = candidates->p + new_capacity;
    }
}

#elif defined(VEC_SIZE_T) || defined(VEC_UINT) //#if defined(VEC_POINTER)

#define VECTOR_INITIALIZE(VECTOR) memset(&(VECTOR), 0, sizeof(VECTOR))
#define VECTOR_FINALIZE(VECTOR) { if ((VECTOR).p != NULL) { DEALLOCATE((VECTOR).p); } }
#define VECTOR_ACCESS(VECTOR, INDEX) (VECTOR).p[INDEX]
#define VECTOR_RAW(VECTOR) (VECTOR).p
#define VECTOR_EMPTY(VECTOR) ((VECTOR).length == 0)
#define VECTOR_LENGTH(VECTOR) ((size_t)(VECTOR).length)
#define VECTOR_RESET(VECTOR) ((VECTOR).length = 0)
#define VECTOR_POP(VECTOR) (VECTOR).length--

#define VECTOR_UNPROTECTED_PUSH(TYPE, VECTOR, ELEMENT) ((VECTOR).p[(VECTOR).length++] = (ELEMENT))
#define VECTOR_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_RESERVE(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_reserve(&(VECTOR), SIZE)

void Connection_vector_push(ConnectionVector *connections, Connection connection)
{
    if (connections->length == connections->capacity)
    {
        const size_t old_capacity = connections->capacity;
        const size_t new_capacity = (old_capacity == 0) ? 1 : (old_capacity << 1);
        REALLOCATE(connections->p, Connection, old_capacity, new_capacity);
        connections->capacity = new_capacity;
    }
    connections->p[connections->length] = connection;
    connections->length += 1;
}

void Benchmark_vector_push(BenchmarkVector *benchmarks, Benchmark benchmark)
{
    if (benchmarks->length == benchmarks->capacity)
    {
        const size_t old_capacity = benchmarks->capacity;
        const size_t new_capacity = (benchmarks->capacity == 0) ? 1 : (benchmarks->capacity << 1);
        REALLOCATE(benchmarks->p, Benchmark, old_capacity, new_capacity);
        benchmarks->capacity = new_capacity;
    }
    benchmarks->p[benchmarks->length] = benchmark;
    benchmarks->length += 1;
}

void ConnectionVector_vector_grow(ConnectionVectorVector *connections, size_t new_size)
{
    if (new_size <= connections->length) return;
    if (new_size > connections->capacity)
    {
        size_t old_size = connections->length;
        size_t new_capacity = (connections->capacity == 0) ? 1 : connections->capacity;
        while (new_size > new_capacity) new_capacity <<= 1;
        REALLOCATE(connections->p, ConnectionVector, old_size, new_capacity);
        connections->capacity = new_capacity;
    }
    connections->length = new_size;
}

void Uint_vector_tight_grow(UintVector *vector, size_t new_size)
{
    if (new_size <= vector->length) return;
    if (new_size <= vector->capacity)
    {
        vector->length = new_size;
    }
    else
    {
        size_t old_size = vector->length;
        REALLOCATE(vector->p, unsigned int, old_size, new_size);
        vector->length = new_size;
        vector->capacity = new_size;
    }
}

void Candidate_vector_tight_reserve(CandidateVector *candidates, size_t new_capacity)
{
    if (new_capacity <= candidates->capacity) {}
    else
    {
        size_t old_size = candidates->length;
        REALLOCATE(candidates->p, Candidate, old_size, new_capacity);
        candidates->capacity = new_capacity;
    }
}

#elif defined(VEC_POINTER_NO_CAPACITY) //#elif defined(VEC_SIZE_T) || defined(VEC_UINT)

#define VECTOR_INITIALIZE(VECTOR) memset(&(VECTOR), 0, sizeof(VECTOR))
#define VECTOR_FINALIZE(VECTOR) { if ((VECTOR).p != NULL) { DEALLOCATE((VECTOR).p); } }
#define VECTOR_ACCESS(VECTOR, INDEX) (VECTOR).p[INDEX]
#define VECTOR_RAW(VECTOR) (VECTOR).p
#define VECTOR_EMPTY(VECTOR) ((VECTOR).end == (VECTOR).p)
#define VECTOR_LENGTH(VECTOR) ((size_t)((VECTOR).end - (VECTOR).p))
#define VECTOR_RESET(VECTOR) ((VECTOR).end = (VECTOR).p)
#define VECTOR_POP(VECTOR) (VECTOR).end--

#define VECTOR_UNPROTECTED_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_RESERVE(TYPE, VECTOR, SIZE)

void Connection_vector_push(ConnectionVector *connections, Connection connection)
{
    const size_t old_size = connections->end - connections->p;
    const size_t new_size = old_size + 1;
    REALLOCATE(connections->p, Connection, old_size, new_size);
    connections->p[old_size] = connection;
    connections->end = connections->p + new_size;
}

void Benchmark_vector_push(BenchmarkVector *benchmarks, Benchmark benchmark)
{
    const size_t old_size = benchmarks->end - benchmarks->p;
    const size_t new_size = old_size + 1;
    REALLOCATE(benchmarks->p, Benchmark, old_size, new_size);
    benchmarks->p[old_size] = benchmark;
    benchmarks->end = benchmarks->p + new_size;
}

void ConnectionVector_vector_grow(ConnectionVectorVector *connections, size_t new_size)
{
    const size_t old_size = connections->end - connections->p;
    REALLOCATE(connections->p, ConnectionVector, old_size, new_size);
    connections->end = connections->p + new_size;
}

void Uint_vector_tight_grow(UintVector *vector, size_t new_size)
{
    const size_t old_size = vector->end - vector->p;
    REALLOCATE(vector->p, unsigned int, old_size, new_size);
    vector->end = vector->p + new_size;
}

void Candidate_vector_push(CandidateVector *candidates, Candidate candidate)
{
    const size_t old_size = candidates->end - candidates->p;
    const size_t new_size = old_size + 1;
    REALLOCATE(candidates->p, Candidate, old_size, new_size);
    candidates->p[old_size] = candidate;
    candidates->end = candidates->p + new_size;
}

#elif defined(VEC_SIZE_T_NO_CAPACITY)

#define VECTOR_INITIALIZE(VECTOR) memset(&(VECTOR), 0, sizeof(VECTOR))
#define VECTOR_FINALIZE(VECTOR) { if ((VECTOR).p != NULL) { DEALLOCATE((VECTOR).p); } }
#define VECTOR_ACCESS(VECTOR, INDEX) (VECTOR).p[INDEX]
#define VECTOR_RAW(VECTOR) (VECTOR).p
#define VECTOR_EMPTY(VECTOR) ((VECTOR).length == 0)
#define VECTOR_LENGTH(VECTOR) ((size_t)(VECTOR).length)
#define VECTOR_RESET(VECTOR) ((VECTOR).length = 0)
#define VECTOR_POP(VECTOR) (VECTOR).length--

#define VECTOR_UNPROTECTED_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_PUSH(TYPE, VECTOR, ELEMENT) TYPE ## _vector_push(&(VECTOR), ELEMENT)
#define VECTOR_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_GROW(TYPE, VECTOR, SIZE) TYPE ## _vector_tight_grow(&(VECTOR), SIZE)
#define VECTOR_TIGHT_RESERVE(TYPE, VECTOR, SIZE)

void Connection_vector_push(ConnectionVector *connections, Connection connection)
{
    const size_t old_size = connections->length;
    const size_t new_size = old_size + 1;
    REALLOCATE(connections->p, Connection, old_size, new_size);
    connections->p[old_size] = connection;
    connections->length = new_size;
}

void Benchmark_vector_push(BenchmarkVector *benchmarks, Benchmark benchmark)
{
    const size_t old_size = benchmarks->length;
    const size_t new_size = old_size + 1;
    REALLOCATE(benchmarks->p, Benchmark, old_size, new_size);
    benchmarks->p[old_size] = benchmark;
    benchmarks->length = new_size;
}

void ConnectionVector_vector_grow(ConnectionVectorVector *connections, size_t new_size)
{
    const size_t old_size = connections->length;
    REALLOCATE(connections->p, ConnectionVector, old_size, new_size);
    connections->length = new_size;
}

void Uint_vector_tight_grow(UintVector *vector, size_t new_size)
{
    const size_t old_size = vector->length;
    REALLOCATE(vector->p, unsigned int, old_size, new_size);
    vector->length = new_size;
}

void Candidate_vector_push(CandidateVector *candidates, Candidate candidate)
{
    const size_t old_size = candidates->length;
    const size_t new_size = old_size + 1;
    REALLOCATE(candidates->p, Candidate, old_size, new_size);
    candidates->p[old_size] = candidate;
    candidates->length = new_size;
}

#elif defined(VEC_UINT_OFFSET)
//TODO
#endif

#endif //#if defined (VEC_CPP)

void push_indexed_heap(CandidateVector *restrict data, unsigned int *restrict indices, Candidate element)
{
    unsigned int i = indices[element.id];
    if (i == (unsigned int)-1)
    {
        i = VECTOR_LENGTH(*data);
        indices[element.id] = i;
        VECTOR_UNPROTECTED_PUSH(Candidate, *data, element);
    }
    else if (i == (unsigned int)-2)
    {
        return;
    }
    else
    {
        if (element.distance < VECTOR_ACCESS(*data, i).distance) VECTOR_ACCESS(*data, i) = element;
        else return;
    }
    while (i > 0)
    {
        const unsigned int parent_i = (i - 1) / 2;
        if (VECTOR_ACCESS(*data, i).distance < VECTOR_ACCESS(*data, parent_i).distance)
        {
            const unsigned int b1 = indices[VECTOR_ACCESS(*data, i).id];
            indices[VECTOR_ACCESS(*data, i).id] = indices[VECTOR_ACCESS(*data, parent_i).id];
            indices[VECTOR_ACCESS(*data, parent_i).id] = b1;

            const Candidate b2 = VECTOR_ACCESS(*data, i);
            VECTOR_ACCESS(*data, i) = VECTOR_ACCESS(*data, parent_i);
            VECTOR_ACCESS(*data, parent_i) = b2;
            
            i = parent_i;
        }
        else break;
    }
}

Candidate pop_indexed_heap(CandidateVector *restrict data, unsigned int *restrict indices)
{
    Candidate top = VECTOR_ACCESS(*data, 0);
    indices[VECTOR_ACCESS(*data, 0).id] = (unsigned int)-2;
    indices[VECTOR_ACCESS(*data, VECTOR_LENGTH(*data) - 1).id] = 0;
    VECTOR_ACCESS(*data, 0) = VECTOR_ACCESS(*data, VECTOR_LENGTH(*data) - 1);
    VECTOR_POP(*data);

    unsigned int i = 0;
    while (true)
    {
        const unsigned int left_i = 2 * i + 1;
        const unsigned int right_i = 2 * i + 2;
        const bool left_exists = left_i < VECTOR_LENGTH(*data);
        const bool right_exists = right_i < VECTOR_LENGTH(*data);
        if (/*left_exists &&*/ right_exists)
        {
            if (VECTOR_ACCESS(*data, left_i).distance < VECTOR_ACCESS(*data, right_i).distance)
            {
                if (VECTOR_ACCESS(*data, left_i).distance < VECTOR_ACCESS(*data, i).distance)
                {
                    const unsigned int b1 = indices[VECTOR_ACCESS(*data, i).id];
                    indices[VECTOR_ACCESS(*data, i).id] = indices[VECTOR_ACCESS(*data, left_i).id];
                    indices[VECTOR_ACCESS(*data, left_i).id] = b1;

                    const Candidate b2 = VECTOR_ACCESS(*data, i);
                    VECTOR_ACCESS(*data, i) = VECTOR_ACCESS(*data, left_i);
                    VECTOR_ACCESS(*data, left_i) = b2;

                    i = left_i;
                }
                else break;
            }
            else
            {
                if (VECTOR_ACCESS(*data, right_i).distance < VECTOR_ACCESS(*data, i).distance)
                {
                    const unsigned int b1 = indices[VECTOR_ACCESS(*data, i).id];
                    indices[VECTOR_ACCESS(*data, i).id] = indices[VECTOR_ACCESS(*data, right_i).id];
                    indices[VECTOR_ACCESS(*data, right_i).id] = b1;

                    const Candidate b2 = VECTOR_ACCESS(*data, i);
                    VECTOR_ACCESS(*data, i) = VECTOR_ACCESS(*data, right_i);
                    VECTOR_ACCESS(*data, right_i) = b2;
                    
                    i = right_i;
                }
                else break;
            }
        }
        else if (left_exists /*&& !right_exists*/)
        {
            if (VECTOR_ACCESS(*data, left_i).distance < VECTOR_ACCESS(*data, i).distance)
            {
                const unsigned int b1 = indices[VECTOR_ACCESS(*data, i).id];
                indices[VECTOR_ACCESS(*data, i).id] = indices[VECTOR_ACCESS(*data, left_i).id];
                indices[VECTOR_ACCESS(*data, left_i).id] = b1;

                const Candidate b2 = VECTOR_ACCESS(*data, i);
                VECTOR_ACCESS(*data, i) = VECTOR_ACCESS(*data, left_i);
                VECTOR_ACCESS(*data, left_i) = b2;

                i = left_i;
            }
            else break;
        }
        else
        {
            break;
        }
    }
    return top;
}

#if defined(IO_CPP_LINE)
void parse(ConnectionVectorVector *graph, BenchmarkVector *benchmarks)
{
    std::ifstream file("dijkstra.txt");
    if (!file.is_open()) throw std::runtime_error("std::ifstream::ifstream() failed");

    bool read_benchmarks = false;
    std::string string;
    while (std::getline(file, string))
    {
        if (string.find("GRAPH") != std::string::npos) { read_benchmarks = false; continue; }
        if (string.find("BENCHMARK") != std::string::npos) { read_benchmarks = true; continue; }
        std::istringstream stream(string);
        if (read_benchmarks)
        {
            unsigned int source, destination;
            stream >> source >> destination;
            if (stream.bad()) break;
            const Benchmark benchmark = { source, destination };
            VECTOR_PUSH(Benchmark, *benchmarks, benchmark);
        }
        else
        {
            unsigned int source, destination;
            float distance;
            stream >> source >> destination >> distance;
            if (stream.bad()) break;
            const unsigned int source_destination_max = (source > destination) ? source : destination;
            if (source_destination_max >= VECTOR_LENGTH(*graph)) VECTOR_GROW(ConnectionVector, *graph, source_destination_max + 1);
            const Connection forward = { destination, distance };
            const Connection backward = { source, distance };
            VECTOR_PUSH(Connection, VECTOR_ACCESS(*graph, source), forward);
            VECTOR_PUSH(Connection, VECTOR_ACCESS(*graph, destination), backward);
        }
    }
}
#elif defined(IO_C_LINE)
void parse(ConnectionVectorVector *graph, BenchmarkVector *benchmarks)
{
    FILE *file = fopen("dijkstra.txt", "r");
    if (file == NULL) { printf("fopen() failed"); exit(2); }

    bool read_benchmarks = false;
    char line[256];
    while (true)
    {
        for (unsigned int i = 0, i_capped = 0;;)
        {
            char c;
            if (fread(&c, 1, 1, file) == 0 || c == '\n' || c == '\r') { line[i_capped] = '\0'; break; }
            line[i_capped] = c;
            i++;
            if (i_capped < sizeof(line) - 1) i_capped++;
        }
        if (strstr(line, "GRAPH") != NULL) { read_benchmarks = false; continue; }
        if (strstr(line, "BENCHMARK") != NULL) { read_benchmarks = true; continue; }
        if (read_benchmarks)
        {
            unsigned int source, destination;
            if (sscanf(line, "%u %u", &source, &destination) != 2) break;
            const Benchmark benchmark = { source, destination };
            VECTOR_PUSH(Benchmark, *benchmarks, benchmark);
        }
        else
        {
            unsigned int source, destination;
            float distance;
            if (sscanf(line, "%u %u %f", &source, &destination, &distance) != 3) break;
            const unsigned int source_destination_max = (source > destination) ? source : destination;
            if (source_destination_max >= VECTOR_LENGTH(*graph)) VECTOR_GROW(ConnectionVector, *graph, source_destination_max + 1);
            const Connection forward = { destination, distance };
            const Connection backward = { source, distance };
            VECTOR_PUSH(Connection, VECTOR_ACCESS(*graph, source), forward);
            VECTOR_PUSH(Connection, VECTOR_ACCESS(*graph, destination), backward);
        }
    }
    fclose(file);
}
#endif

void solve(const ConnectionVectorVector *restrict graph, const BenchmarkVector *restrict benchmarks)
{
    CandidateVector candidates;
    UintVector indices;
    VECTOR_INITIALIZE(candidates);
    VECTOR_INITIALIZE(indices);
    VECTOR_TIGHT_RESERVE(Candidate, candidates, VECTOR_LENGTH(*graph));
    VECTOR_TIGHT_GROW(Uint, indices, VECTOR_LENGTH(*graph));

    for (size_t benchmark_i = 0; benchmark_i < VECTOR_LENGTH(*benchmarks); benchmark_i++)
    {
        const unsigned int source = VECTOR_ACCESS(*benchmarks, benchmark_i).source;
        const unsigned int destination = VECTOR_ACCESS(*benchmarks, benchmark_i).destination;
        VECTOR_RESET(candidates);
        for (size_t i = 0; i < VECTOR_LENGTH(indices); i++) VECTOR_ACCESS(indices, i) = (unsigned int)-1;
        const Candidate first_candidate = { source, 0, 0.0 };
        push_indexed_heap(&candidates, VECTOR_RAW(indices), first_candidate);
        unsigned int int_distance = 0;
        float distance = std::numeric_limits<float>::infinity();
        while (!VECTOR_EMPTY(candidates))
        {
            Candidate candidate = pop_indexed_heap(&candidates, VECTOR_RAW(indices));
            if (candidate.id == destination) { int_distance = candidate.int_distance; distance = candidate.distance; break; }
            const ConnectionVector *connections = &VECTOR_ACCESS(*graph, candidate.id);
            for (size_t i = 0; i < VECTOR_LENGTH(*connections); i++)
            {
                const Candidate new_candidate = {
                    VECTOR_ACCESS(*connections, i).destination,
                    candidate.int_distance + 1,
                    candidate.distance + VECTOR_ACCESS(*connections, i).distance
                };
                push_indexed_heap(&candidates, VECTOR_RAW(indices), new_candidate);
            }
        }
        #if defined(CMP_GPP) || defined(CMP_LLVMPP)
            std::cout << source << " -> " << destination << ": " << distance << " (" << int_distance << ")" << std::endl;
        #else
            printf("%u -> %u: %f (%u)\n", source, destination, distance, int_distance);
        #endif
    }
}

int _main(timespec *middle)
{
    ConnectionVectorVector graph;
    BenchmarkVector benchmarks;
    VECTOR_INITIALIZE(graph);
    VECTOR_INITIALIZE(benchmarks);
    parse(&graph, &benchmarks);
    clock_gettime(CLOCK_MONOTONIC, middle);
    solve(&graph, &benchmarks);
    for (size_t i = 0; i < VECTOR_LENGTH(graph); i++) VECTOR_FINALIZE(VECTOR_ACCESS(graph, i));
    VECTOR_FINALIZE(graph);
    VECTOR_FINALIZE(benchmarks);
    return 0;
}

int main()
{
    #if defined(CMP_GPP) || defined(CMP_LLVMPP)
    try
    {
    #endif
        timespec begin, middle, end;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        int code = _main(&middle);
        clock_gettime(CLOCK_MONOTONIC, &end);
        while (begin.tv_sec < middle.tv_sec) { middle.tv_sec--; middle.tv_nsec += 1000 * 1000 * 1000; }
        while (begin.tv_sec < end.tv_sec) { end.tv_sec--; end.tv_nsec += 1000 * 1000 * 1000; }
        #if defined(CMP_GPP) || defined(CMP_LLVMPP)
            std::cerr << "parse   " << (middle.tv_nsec - begin.tv_nsec) << "ns" << std::endl;
            std::cerr << "solve   " << (end.tv_nsec - middle.tv_nsec) << "ns" << std::endl;
        #else
            fprintf(stderr, "parse   %uns\n", (middle.tv_nsec - begin.tv_nsec));
            fprintf(stderr, "solve   %uns\n", (end.tv_nsec - middle.tv_nsec));
        #endif
    #if defined(CMP_GPP) || defined(CMP_LLVMPP)
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    #endif
}
