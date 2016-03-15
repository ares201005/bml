#include "../bml_introspection.h"
#include "../bml_logger.h"
#include "bml_setters_ellpack.h"
#include "bml_types_ellpack.h"

void
bml_set_element_ellpack(
    bml_matrix_ellpack_t * A,
    const int i,
    const int j,
    const void *value)
{
    switch (bml_get_precision(A))
    {
        case single_real:
            bml_set_element_ellpack_single_real(A, i, j, value);
            break;
        case double_real:
            bml_set_element_ellpack_double_real(A, i, j, value);
            break;
        case single_complex:
            bml_set_element_ellpack_single_complex(A, i, j, value);
            break;
        case double_complex:
            bml_set_element_ellpack_double_complex(A, i, j, value);
            break;
        default:
            LOG_ERROR("unkonwn precision in bml_set_element_ellpack\n");
            break;
    }
}

void
bml_set_row_ellpack(
    bml_matrix_ellpack_t * A,
    const int i,
    const void *row,
    const double threshold)
{
    switch (A->matrix_precision)
    {
        case single_real:
            bml_set_row_ellpack_single_real(A, i, row, threshold);
            break;
        case double_real:
            bml_set_row_ellpack_double_real(A, i, row, threshold);
            break;
        case single_complex:
            bml_set_row_ellpack_single_complex(A, i, row, threshold);
            break;
        case double_complex:
            bml_set_row_ellpack_double_complex(A, i, row, threshold);
            break;
        default:
            LOG_ERROR("unkonwn precision\n");
            break;
    }
}

void
bml_set_diagonal_ellpack(
    bml_matrix_ellpack_t * A,
    const void *diagonal,
    const double threshold)
{
    switch (A->matrix_precision)
    {
        case single_real:
            bml_set_diagonal_ellpack_single_real(A, diagonal, threshold);
            break;
        case double_real:
            bml_set_diagonal_ellpack_double_real(A, diagonal, threshold);
            break;
        case single_complex:
            bml_set_diagonal_ellpack_single_complex(A, diagonal, threshold);
            break;
        case double_complex:
            bml_set_diagonal_ellpack_double_complex(A, diagonal, threshold);
            break;
        default:
            LOG_ERROR("unkonwn precision\n");
            break;
    }
}
