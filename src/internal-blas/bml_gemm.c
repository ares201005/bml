#include "../macros.h"
#include "../typed.h"
#include "../C-interface/blas.h"
#include "../C-interface/bml_logger.h"

#include <complex.h>
#include <stdio.h>

void TYPED_FUNC(
    bml_gemm_internal) (
    char *transa,
    char *transb,
    int *m,
    int *n,
    int *k,
    REAL_T * alpha,
    REAL_T * a,
    int *lda,
    REAL_T * b,
    int *ldb,
    REAL_T * beta,
    REAL_T * c,
    int *ldc)
{
    /* Reference implementation from
     * http://www.netlib.org/lapack/explore-html/d1/d54/group__double__blas__level3_gaeda3cbd99c8fb834a60a6412878226e1.html
     */

    int m_val = *m;
    int n_val = *n;
    int k_val = *k;
    REAL_T alpha_val = *alpha;
    REAL_T beta_val = *beta;

    int N_rows_A;
    int N_rows_B;
    int N_cols_A;

    if (*transa == 'N')
    {
        N_rows_A = m_val;
        N_cols_A = k_val;
    }
    else
    {
        N_rows_A = k_val;
        N_cols_A = m_val;
    }

    if (*transb == 'N')
    {
        N_rows_B = k_val;
    }
    else
    {
        N_rows_B = n_val;
    }

    int info = 0;

    if (*transa != 'N' && *transa != 'C' && *transa != 'T')
    {
        info = 1;
    }
    else if (*transb != 'N' && *transb != 'C' && *transb != 'T')
    {
        info = 2;
    }
    else if (m_val < 0)
    {
        info = 3;
    }
    else if (n_val < 0)
    {
        info = 4;
    }
    else if (k_val < 0)
    {
        info = 5;
    }
    else if (*lda < MAX(1, N_rows_A))
    {
        info = 8;
    }
    else if (*ldb < MAX(1, N_rows_B))
    {
        info = 10;
    }
    else if (*ldc < MAX(1, m_val))
    {
        info = 13;
    }

    if (info != 0)
    {
        /* Error. */
        LOG_ERROR("info = %d\n", info);
        return;
    }

    if ((m_val == 0 || n_val == 0)
        || ((alpha_val == 0 || k_val == 0) && beta_val == 1.0))
    {
        return;
    }

    if (alpha_val == 0)
    {
        if (beta_val == 0)
        {
	//#pragma omp target data map(tofrom:c[:m_val*n_val])	
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
            for (int j = 0; j < n_val; j++)
            {
                for (int i = 0; i < m_val; i++)
                {
                    c[COLMAJOR(i, j, m_val, n_val)] = 0;
                }
            }
        }
        else
        {
	//#pragma omp target data map(tofrom:c[:m_val*n_val])
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
            for (int j = 0; j < n_val; j++)
            {
                for (int i = 0; i < m_val; i++)
                {
                    c[COLMAJOR(i, j, m_val, n_val)] =
                        beta_val * c[COLMAJOR(i, j, m_val, n_val)];
                }
            }
        }
        return;
    }
    if (*transb == 'N')
    {
        if (*transa == 'N')
        {
            /* C := alpha*A*B + beta*C
             */
            int i, j, l;

	    //#pragma omp target data map(to:a[:m_val*k_val],b[:n_val*k_val]) map(tofrom:c[:m_val*n_val])
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
            for (j = 0; j < n_val; j++)
            {
                {
                    for (i = 0; i < m_val; i++)
                    {
                        c[COLMAJOR(i, j, m_val, n_val)] *= beta_val;
                        for (l = 0; l < k_val; l++)
                        {
                            c[COLMAJOR(i, j, m_val, n_val)] +=
                                alpha_val * a[COLMAJOR(i, l, m_val, k_val)] *
                                b[COLMAJOR(l, j, k_val, n_val)];
                        }
                    }
                }
            }
        }
        else
        {
            /* C := alpha*A**T*B + beta*C
             */

	//#pragma omp target data map(to:a[:m_val*k_val],b[:n_val*k_val]) map(tofrom:c[:m_val*n_val])
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
            for (int j = 0; j < n_val; j++)
            {
                {
                    for (int i = 0; i < m_val; i++)
                    {
                        c[COLMAJOR(i, j, m_val, n_val)] *= beta_val;
                        for (int l = 0; l < k_val; l++)
                        {
                            c[COLMAJOR(i, j, m_val, n_val)] +=
                                alpha_val * a[COLMAJOR(l, i, k_val, m_val)] *
                                b[COLMAJOR(l, j, k_val, n_val)];
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (*transa == 'N')
        {
            /* C := alpha*A*B**T + beta*C
             */


	//#pragma omp target data map(to:a[:m_val*k_val],b[:n_val*k_val]) map(tofrom:c[:m_val*n_val])
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
	//#pragma omp target
            for (int j = 0; j < n_val; j++)
            {
                {
                    for (int i = 0; i < m_val; i++)
                    {
                        c[COLMAJOR(i, j, m_val, n_val)] *= beta_val;
                        for (int l = 0; l < k_val; l++)
                        {
                            c[COLMAJOR(i, j, m_val, n_val)] +=
                                alpha_val * a[COLMAJOR(i, l, m_val, k_val)] *
                                b[COLMAJOR(j, l, n_val, k_val)];
                        }
                    }
                }
            }
        }
        else
        {
            /* C := alpha*A**T*B**T + beta*C
             */

	//#pragma omp target data map(to:a[:m_val*k_val],b[:n_val*k_val]) map(tofrom:c[:m_val*n_val])
#pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 1)
            for (int j = 0; j < n_val; j++)
            {
                {
                    for (int i = 0; i < m_val; i++)
                    {
                        c[COLMAJOR(i, j, m_val, n_val)] *= beta_val;
                        for (int l = 0; l < k_val; l++)
                        {
                            c[COLMAJOR(i, j, m_val, n_val)] +=
                                alpha_val * a[COLMAJOR(l, i, k_val, m_val)] *
                                b[COLMAJOR(j, l, n_val, k_val)];
                        }
                    }
                }
            }
        }
    }
}

void TYPED_FUNC(
    bml_gemm) (
    char *transa,
    char *transb,
    int *m,
    int *n,
    int *k,
    REAL_T * alpha,
    REAL_T * a,
    int *lda,
    REAL_T * b,
    int *ldb,
    REAL_T * beta,
    REAL_T * c,
    int *ldc)
{
#ifdef BML_INTERNAL_GEMM
    int MN = *m * *n;
    TYPED_FUNC(bml_gemm_internal) (transa, transb, m, n, k, alpha, a,
                                   lda, b, ldb, beta, c, ldc);
#pragma omp target update from(c[:MN])
#else

#ifdef NOBLAS
    LOG_ERROR("No BLAS library");
#else
    C_BLAS(GEMM) (transa, transb, m, n, k, alpha, a,
                  lda, b, ldb, beta, c, ldc);
#endif
#endif
}
