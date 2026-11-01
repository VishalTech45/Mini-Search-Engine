# Mini Search Engine — Algorithms & Complexity Analysis

## 1. Algorithmic Foundations

This document details the mathematical models, algorithmic mechanisms, and time/space complexity analysis underlying the Mini Search Engine.

---

## 2. Text Preprocessing & Canonicalization

### 2.1 Lexical Transformation Pipeline
1. **Case Folding**: Each ASCII byte $c$ is converted to lowercase: $c \leftarrow \text{tolower}(c)$.
2. **Punctuation Normalization**: Characters satisfying `std::ispunct(c)` are converted to whitespace (`' '`), except when `+` is part of `"c++"`.
3. **Whitespace Tokenization**: Extracted strings are separated along whitespace boundaries into a token stream $T = [t_1, t_2, \dots, t_m]$.
4. **Stop-Word Removal**: Given a vocabulary of stop words $S$, each token $t_i \in S$ is discarded, yielding filtered tokens $T'$. If $|T'| = 0$, $T'$ falls back to $T$ to avoid discarding queries consisting solely of function words.

### 2.2 Query Canonicalization
To enforce consistent popularity tracking across word order variations:
$$\text{CanonicalKey}(Q) = \text{JoinWithSpaces}(\text{Sort}(\text{Unique}(T')))$$
Example: Both `"machine learning"` and `"learning machine"` reduce deterministically to the identical key `"learning machine"`.

---

## 3. Inverted Index Architecture & Query Processing

### 3.1 Postings Structure
Each vocabulary term $t$ maps to a postings list:
$$\text{Index}[t] = [P_1, P_2, \dots, P_k], \quad P_i = \langle \text{docId}_i, \text{termFreq}_i \rangle$$
Postings lists are maintained strictly sorted in ascending order of `docId`.

### 3.2 Conjunctive Search (`AND` Mode)
Given query terms $Q = \{t_1, t_2, \dots, t_q\}$:
- Retrieve postings lists $L_1, L_2, \dots, L_q$.
- If any term $t_i \notin \text{Vocabulary}$, return $\emptyset$ immediately.
- Compute the set intersection using a two-pointer merge algorithm:
  $$\text{Candidates} = L_1 \cap L_2 \cap \dots \cap L_q$$
- Time complexity: $O\left(\sum_{i=1}^{q-1} \min(|L_i|, |L_{i+1}|)\right)$.

### 3.3 Disjunctive Search (`OR` Mode)
- Retrieve postings lists for all present query terms.
- Compute the set union using an in-memory hash set or multiway merge:
  $$\text{Candidates} = L_1 \cup L_2 \cup \dots \cup L_q$$
- Time complexity: $O\left(\sum_{i=1}^q |L_i|\right)$.

---

## 4. Ranking Formula & Mathematical Derivation

The Mini Search Engine uses a hybrid ranking algorithm combining statistical text relevance ($\text{TF-IDF}$) with empirical user interaction history ($\text{Popularity}$):

### 4.1 Term Frequency ($\text{TF}$)
To penalize term repetition in disproportionately long documents:
$$\text{TF}(t, d) = \frac{\text{freq}(t, d)}{|d|}$$
where $\text{freq}(t, d)$ is the count of term $t$ in document $d$, and $|d|$ is the total word count of $d$.

### 4.2 Inverse Document Frequency ($\text{IDF}$)
Using the smoothed logarithmic formulation:
$$\text{IDF}(t) = \ln\left(1.0 + \frac{N}{\text{DF}(t)}\right)$$
where $N$ is total documents in the index, and $\text{DF}(t)$ is document frequency ($|\text{Index}[t]|$).
- Properties:
  - If a term appears in all documents ($\text{DF}(t) = N$), $\text{IDF}(t) = \ln(2) \approx 0.693$.
  - If a term appears in only 1 out of 1000 documents, $\text{IDF}(t) = \ln(1001) \approx 6.909$, automatically amplifying distinctive discriminative keywords.

### 4.3 Raw Relevance Score
$$\text{RawRelevance}(Q, d) = \sum_{t \in Q} \text{TF}(t, d) \times \text{IDF}(t)$$

### 4.4 Score Normalization
To prevent raw numerical magnitudes from distorting the popularity balance:
$$\text{NormRelevance}(d) = \begin{cases} \frac{\text{RawRelevance}(d)}{\max_{d'} \text{RawRelevance}(d')}, & \text{if } \max > 0 \\ 0, & \text{otherwise} \end{cases}$$
$$\text{NormPopularity}(Q, d) = \begin{cases} \frac{\text{Visits}(Q, d)}{\max_{d'} \text{Visits}(Q, d')}, & \text{if } \max > 0 \\ 0, & \text{otherwise} \end{cases}$$

### 4.5 Composite Final Ranking Score
$$\text{FinalScore}(Q, d) = (\alpha \times \text{NormRelevance}(d)) + (\beta \times \text{NormPopularity}(Q, d))$$
With default weights $\alpha = 0.8$ and $\beta = 0.2$:
- **Relevance Dominance**: Because $\alpha = 0.8$ and $\beta = 0.2$, a document with low relevance ($0.2$) cannot surpass a highly relevant document ($1.0$) purely through artificial clicks ($0.8 \times 0.2 + 0.2 \times 1.0 = 0.36 < 0.80$).

---

## 5. Comprehensive Time & Space Complexity Analysis

Let:
- $N$ = Total number of documents.
- $L$ = Average document character length.
- $W$ = Average number of words per document.
- $V$ = Total vocabulary size across all documents.
- $Q_w$ = Number of words in user query.
- $K$ = Number of candidate matching documents ($K \le N$).
- $U$ = Total number of unique queries in persistence.

| Operation | Average Time Complexity | Worst-Case Time Complexity | Space Complexity | Explanation |
|---|---|---|---|---|
| **1. Document Loading** | $O(N \times L)$ | $O(N \times L)$ | $O(N \times L)$ | Traverses directory and reads each file buffer into heap memory. |
| **2. Text Tokenization** | $O(L)$ | $O(L)$ | $O(W)$ | Linear pass over character string, generating vector of tokens. |
| **3. Inverted Index Construction** | $O(N \times W)$ | $O(N \times W + N \times V)$ | $O(V + N \times W)$ | Evaluates local term frequencies and populates hash map postings lists. |
| **4. Single-Keyword Search** | $O(1 + |P|)$ | $O(V + |P|)$ | $O(1)$ average hash table lookup; extracts posting list of length $|P|$. |
| **5. Multi-Keyword Search (OR)** | $O(Q_w + K)$ | $O(Q_w \times V + K \log K)$ | $O(K)$ | Unions postings across $Q_w$ terms and deduplicates document IDs. |
| **6. Multi-Keyword Search (AND)** | $O(Q_w \times |P_{\min}|)$ | $O(Q_w \times N)$ | $O(K)$ | Two-pointer intersection along sorted postings lists. |
| **7. TF-IDF Calculation** | $O(K \times Q_w)$ | $O(K \times Q_w)$ | $O(K)$ | Lookups of term frequencies and arithmetic aggregation for $K$ candidates. |
| **8. Result Ranking** | $O(K \log K)$ | $O(K \log K)$ | $O(1)$ auxiliary | Introsort (`std::sort`) executes hybrid quicksort/heapsort in-place. |
| **9. Popularity Updates** | $O(1)$ | $O(U)$ | $O(1)$ | Nested hash table insertion/incrementation: `queryDocVisits[q][docId]++`. |
| **10. Persistence Save (JSON)** | $O(U \times K_{\text{avg}})$ | $O(U \times N)$ | $O(U \times K_{\text{avg}})$ | Serializes nested map to temporary disk file and atomically renames. |
| **11. Persistence Load (JSON)** | $O(S_{\text{file}})$ | $O(S_{\text{file}})$ | $O(U \times K_{\text{avg}})$ | Lexes and parses JSON byte stream into memory. |
