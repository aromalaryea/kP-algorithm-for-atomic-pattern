#include "F28x_Project.h"
#include "driverlib.h"
#include "device.h"
//
// Main
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flecc_in_c/types.h"
#include "flecc_in_c/io/io.h"
#include "flecc_in_c/bi/bi.h"
#include "flecc_in_c/eccp/eccp.h"
#include "flecc_in_c/utils/param.h"
#include "flecc_in_c/utils/parse.h"
#include "flecc_in_c/gfp/gfp.h"

//curve P-256
const char *curveTypeStr = "secp256r1";  // Curve type, P-256 elliptic curve

//constant R^2, needed for montgomery multiplications
//Because CCS cannot compute this value properly
const char *RSquared = "4FFFFFFFDFFFFFFFFFFFFFFFEFFFFFFFBFFFFFFFF0000000000000003";

// Q - Static Coordinates
const char *q0xStr = "6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296"; // X1 (Affine x-coordinate of point Q):
const char *q0yStr = "4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5"; //Y1 (Affine y-coordinate of point Q):
const char *q0zStr = "0000000000000000000000000000000000000000000000000000000000000001"; // Z2 (Projective coordinate of point Q): (It's always 1 for the identity element)
const char *q0wStr = "0000000000000000000000000000000000000000000000000000000000000003"; //W1 az1^4 = -3, where a=-3

const char *q_tempStr = "0";

// Scalar value
const char *kBSrc = "1111111111";  // Scalar value (2342397)

const gfp_prime_data_t *primeNumber; // Prime Number used for reduction
const gfp_prime_data_t *primeWords;  // length of the primeNumber
eccp_parameters_t curveParameters;  // Curve parameters

//Registers Initialization
gfp_t R0, R1, R2, R3, R4, R5, R6;

gfp_t tempReg; //temporary Registry for computing dummy operations and storing temp values
gfp_t aVal; //
gfp_t rsquared; // constant R^2, because CCS cannot compute this value properly

gfp_t q0x; //X1
gfp_t q0y; //Y1
gfp_t q0z; //Z1
gfp_t q0w; //W1

gfp_t p0x; //X2
gfp_t p0y; //Y2
gfp_t p0z; //Z2

gfp_t r0x; //X2
gfp_t r0y; //Y2
gfp_t r0z; //Z2
gfp_t r0w; //W1

gfp_t qx_temp;
gfp_t qy_temp;
gfp_t qz_temp;

//Transition to Affinates Cordinates
gfp_t ZTrans; //To compute the Z in transitioning
gfp_t result_X; //Affine cordinate for X
gfp_t result_Y; //Affine cordinate for Y

// 1st Function to add two points on an elliptic curve
void ec_PointAddition1(gfp_t X1, gfp_t X2, gfp_t Y1, gfp_t Y2, gfp_t Z1, gfp_t Z2){

    //OP 1   R1 <- Z2 * Z2
    gfp_mult_two_mont(R1, Z2, Z2, primeNumber, rsquared);

    //OP 2 dummy operation tempReg <- R1 + Z1
    gfp_cr_add(tempReg, R1, Z1, primeNumber);

    //OP 3 R2 <- Y1 * Z2
    gfp_mult_two_mont(R2, Y1, Z2, primeNumber, rsquared);

    //OP 4 dummy operation tempReg <- R2 + Z2
    gfp_cr_add(tempReg, R2, Z2, primeNumber);

    //OP 5 R5 <- Y2 * Z1
    gfp_mult_two_mont(R5, Y2, Z1, primeNumber, rsquared);

    //OP 6 dummy operation tempReg <- R2 + Z2
    gfp_cr_add(tempReg, R2, Z2, primeNumber);

    //OP 7 R3 <- R1 * R2
    gfp_mult_two_mont(R3, R1, R2, primeNumber, rsquared);

    //OP 8 dummy operation tempReg <- R3 + R1
    gfp_cr_add(tempReg, R3, R1, primeNumber);

    //OP 9 dummy operation tempReg <- R5 + Z2
    gfp_cr_add(tempReg, R5, Z2, primeNumber);

    //OP 10 R4 <- Z1^2
    gfp_mult_two_mont(R4, Z1, Z1, primeNumber, rsquared);

    //OP 11 R2 <- R5 * R4
    gfp_mult_two_mont(R2, R5, R4, primeNumber, rsquared);

    //OP 12 dummy operation tempReg <- R2 + R4
    gfp_cr_add(tempReg, R2, R4, primeNumber);

    //OP 13 R2 <- R2 - R3
    gfp_cr_subtract(R2, R2, R3, primeNumber);

    //OP 14 R5 <- R1 * X1
    gfp_mult_two_mont(R5, R1, X1, primeNumber, rsquared);

    //OP 15 dummy operation tempReg <- R5 - X1
    gfp_cr_subtract(tempReg, R5, X1, primeNumber);

    //OP 16 dummy operation tempReg <- R4 - X2
    gfp_cr_subtract(tempReg, R4, X2, primeNumber);

    //OP 17 R6 <- X2 * R4
    gfp_mult_two_mont(R6, X2, R4, primeNumber, rsquared);

    //OP 18 R6 <- R6 - R5
    gfp_cr_subtract(R6, R6, R5, primeNumber);

}

// 2nd Function to add two points on an elliptic curve
void ec_PointAddition2(gfp_t X3, gfp_t Y3, gfp_t Z1, gfp_t Z2, gfp_t Z3){

    //OP 1   R1 <- R6^2
    gfp_mult_two_mont(R1, R6, R6, primeNumber, rsquared);

    //OP 2 dummy operation tempReg <- R2 + R3
    gfp_cr_add(tempReg, R2, R3, primeNumber);

    //OP 3 R4 <- R5 * R1
    gfp_mult_two_mont(R4, R5, R1, primeNumber, rsquared);

    //OP 4 dummy operation tempReg <- R4 + R3
    gfp_cr_add(tempReg, R4, R3, primeNumber);

    //OP 5 R5 <- R1 * R6
    gfp_mult_two_mont(R5, R1, R6, primeNumber, rsquared);

    //OP 6 dummy operation tempReg <- R5 + R6
    gfp_cr_add(tempReg, R5, R6, primeNumber);

    //OP 7 R1 <- Z1 * R6
    gfp_mult_two_mont(R1, Z1, R6, primeNumber, rsquared);

    //OP 8 dummy operation tempReg <- R1 + Z1
    gfp_cr_add(tempReg, R1, Z1, primeNumber);

    //OP 9 dummy operation tempReg <- R6 + R2
    gfp_cr_add(tempReg, R6, R2, primeNumber);

    //OP 10 R6 <- R2^2
    gfp_mult_two_mont(R6, R2, R2, primeNumber, rsquared);

    //OP 11 Z3 <- R1 * Z2
    gfp_mult_two_mont(Z3, R1, Z2, primeNumber, rsquared);

    //OP 12 R1 <- R4 + R4
    gfp_cr_add( R1, R4, R4, primeNumber);

    //OP 13 R6 <- R6 - R1
    gfp_cr_subtract( R6, R6, R1, primeNumber);

    //OP 14 R1 <- R5 * R3
    gfp_mult_two_mont(R1, R5, R3, primeNumber, rsquared);

    //OP 15 X3 <- R6 - R5
    gfp_cr_subtract( X3, R6, R5, primeNumber);

    //OP 16 R4 <- R4 -X3
    gfp_cr_subtract( R4, R4, X3, primeNumber);

    //OP 17 R3 <- R4 * R2
    gfp_mult_two_mont(R3, R4, R2, primeNumber, rsquared);

    //OP 18 Y3 <- R3 - R1
    gfp_cr_subtract( Y3, R3, R1, primeNumber);

}

// Function to double a point on an elliptic curve
void ec_PointDoubling(gfp_t X1, gfp_t X2, gfp_t Y1, gfp_t Y2, gfp_t Z1, gfp_t Z2, gfp_t W1, gfp_t W2){

    //OP 1 R1 <- X1^2
    gfp_mult_two_mont(R1, X1, X1, primeNumber, rsquared);

    //OP 2 R2 <- Y1 +Y1
    gfp_cr_add(R2, Y1, Y1, primeNumber) ;

    //OP 3 Z2 <- R2 * Z1
    gfp_mult_two_mont(Z2, R2, Z1, primeNumber, rsquared);

    //OP 4 R4 <- R1 + R1
    gfp_cr_add(R4, R1, R1, primeNumber) ;

    //OP 5 R3 <- R2 * Y1
    gfp_mult_two_mont(R3, R2, Y1, primeNumber, rsquared);

    //OP 6 R6 <- R3 + R3
    gfp_cr_add(R6, R3, R3, primeNumber) ;

    //OP 7 R2 <- R6 * R3
    gfp_mult_two_mont(R2, R6, R3, primeNumber, rsquared);

    //OP 8 R1 <- R4 + R1
    gfp_cr_add(R1, R4, R1, primeNumber) ;

    //OP 9 R1 <- R1 + W1
    gfp_cr_add(R1, R1, W1, primeNumber) ;

    //OP 10 R3 <- R1^2
    gfp_mult_two_mont(R3, R1, R1, primeNumber, rsquared);

    //OP 11 R4 <- R6 * X1
    gfp_mult_two_mont(R4, R6, X1, primeNumber, rsquared);

    //OP 12 R5 <- W1 + W1
    gfp_cr_add(R5, W1, W1, primeNumber);

    //OP 13 R3 <- R3 - R4
    gfp_cr_subtract(R3, R3, R4, primeNumber);

    //OP 14 W2 <- R2 * R5
    gfp_mult_two_mont(W2, R2, R5, primeNumber, rsquared);

    //OP 15 X2 <- R3 - R4
    gfp_cr_subtract(X2, R3, R4, primeNumber);

    //OP 16 R6 <- R4 -X2
    gfp_cr_subtract(R6, R4, X2, primeNumber);

    //OP 17 R4 <- R6 * R1
    gfp_mult_two_mont(R4, R6, R1, primeNumber, rsquared);

    //OP 18 Y2 <- R4 - R2
    gfp_cr_subtract(Y2, R4, R2, primeNumber);

}

void parse_bigint( const char *string, uint_t *big_int, const int bi_length ) {
    int len = strlen( string );
    bigint_parse_hex_var( big_int, bi_length, string, len );
}

void nop_delay(int cycles) {
    int dummy = 0; // Arbitrary variable to perform operations on
    for (int j = 0; j < cycles; j++) {
        dummy += j; // Arbitrary operation to ensure loop does some work
    }
}

void TransitionToAffine(gfp_t q0x ,gfp_t q0y, gfp_t q0z, const char *scalarBinary, int scalarLength){

    //if (scalarBinary[scalarLength - 1] == '1') {

        //Affine X Cordinate i.e. X = X1/Z3^2
        gfp_mult_two_mont(ZTrans, q0z, q0z, primeNumber, rsquared); //Z3 * Z3
        gfp_binary_euclidean_inverse(result_X, ZTrans, primeNumber);// 1/(Z3 * Z3)
        gfp_mult_two_mont(result_X, q0x, result_X, primeNumber, rsquared); //X1 * (1/(Z3 * Z3))

        //Affine Y Cordinate Y = Y1/Z3^3
        gfp_mult_two_mont(ZTrans, q0z, ZTrans, primeNumber, rsquared);//Z3*Z3*Z3
        gfp_binary_euclidean_inverse(result_Y, ZTrans, primeNumber);// 1/(Z3*Z3*Z3)
        gfp_mult_two_mont(result_Y, q0y, result_Y, primeNumber, rsquared);// Y1 * (1/(Z3*Z3*Z3))

        printf("R2L Final Affine Cordinates:");
        printf("\n");
        printf("Affine_X: ");
        io_print_bigint_var(result_X, primeWords);
        printf("Affine_Y: ");
        io_print_bigint_var(result_Y, primeWords);
    //}

}

void RightToLeftBinaryScalarMultiply(gfp_t q0x, gfp_t q0y, gfp_t q0z, gfp_t r0x, gfp_t r0y, gfp_t r0z, gfp_t r0w, gfp_t qx_temp, gfp_t qy_temp, gfp_t qz_temp, const char *scalarBinary) {
    int scalarLength = strlen(scalarBinary);
    int i = scalarLength - 2;

    // Initialize Q to the input point: Q = 0
    bigint_copy_var(q0x, qx_temp, primeWords); //ensure p0x = q0x; copy content of qx_temp into q0x
    bigint_copy_var(q0y, qy_temp, primeWords); //ensure p0x = q0x; copy content of qy_temp into q0y
    bigint_copy_var(q0z, qz_temp, primeWords); //ensure p0x = q0x; copy content of qz_temp into q0z

    // Process the first bit separately to initialize R: Q=Q+R=P and R=2R=2P
    if (scalarBinary[scalarLength - 1] == '1' || scalarBinary[scalarLength - 1] == '0') {
        ec_PointAddition1(q0x, r0x, q0y, r0y, q0z, r0z); //X1,X2,Y1,Y2,Z1,Z2
        nop_delay(1000); //NOPs loop between PA1 and PA2
        ec_PointAddition2(q0x, q0y, q0z, r0z, q0z); //X3,Y3,Z1,Z2,Z3

        bigint_copy_var(q0x, r0x, primeWords); //ensure p0x = q0x; copy content of r0x into q0x
        bigint_copy_var(q0y, r0y, primeWords); //ensure p0y = q0y; copy content of r0y into q0y
        bigint_copy_var(q0z, r0z, primeWords); //ensure p0y = q0y; copy content of r0z into q0z

        nop_delay(2000); //NOPs loop before PD
        ec_PointDoubling(r0x, r0x, r0y, r0y, r0z, r0z, r0w, r0w);
        nop_delay(3000); //NOPs loop after PD
    }

    // Loop through the binary scalar bits from right to left
    for (i = scalarLength - 2; i >= 0; i--) {
        if (scalarBinary[i] == '1') {

            ec_PointAddition1(q0x, r0x, q0y, r0y, q0z, r0z); //X1,X2,Y1,Y2,Z1,Z2
            nop_delay(1000); //NOPs loop between PA1 and PA2
            ec_PointAddition2(q0x, q0y, q0z, r0z, q0z); //X3,Y3,Z1,Z2,Z3

        }
        nop_delay(2000); //NOPs loop before PD
        ec_PointDoubling(r0x, r0x, r0y, r0y, r0z, r0z, r0w, r0w);
        nop_delay(3000); //NOPs loop after PD
    }

    printf("R2L Final Projective Coordinates:\n");
    printf("Projective_X: ");
    io_print_bigint_var(q0x, primeWords);
    printf("Projective_Y: ");
    io_print_bigint_var(q0y, primeWords);
    printf("Projective_Z: ");
    io_print_bigint_var(q0z, primeWords);
    printf("\n");

    // Transition to affine coordinates
    TransitionToAffine(q0x, q0y, q0z, scalarBinary, scalarLength);
}

int main(void){

    Device_init();

    // Load curve parameters
    curveParameters.curve_type = param_get_curve_type_from_name(curveTypeStr, strlen(curveTypeStr));
    param_load(&curveParameters, curveParameters.curve_type);

    primeNumber = curveParameters.prime_data.prime;
    primeWords = curveParameters.order_n_data.words;

    // Parse (convert from hex to int) and initialize input values
    parse_bigint(q0xStr, q0x, primeWords);//convert the hex string from q0xStr into a big integer and save in q0x
    parse_bigint(q0yStr, q0y, primeWords);//convert the hex string from q0yStr into a big integer and save in q0y
    parse_bigint(q0zStr, q0z, primeWords);//convert the hex string from q0zStr into a big integer and save in q0z
    parse_bigint(q0wStr, q0w, primeWords);//convert the hex string from q0zStr into a big integer and save in q0z
    gfp_cr_negate(q0w, q0w, primeNumber);//negate W1 and save it in q0w

    parse_bigint(q0xStr, p0x, primeWords);//convert the hex string from q0xStr into a big integer and save in p0x
    parse_bigint(q0yStr, p0y, primeWords);//convert the hex string from q0yStr into a big integer and save in p0y
    parse_bigint(q0zStr, p0z, primeWords);//convert the hex string from q0zStr into a big integer and save in p0z

    parse_bigint(q0xStr, r0x, primeWords);//convert the hex string from q0xStr into a big integer and save in r0x
    parse_bigint(q0yStr, r0y, primeWords);//convert the hex string from q0yStr into a big integer and save in r0y
    parse_bigint(q0zStr, r0z, primeWords);//convert the hex string from q0zStr into a big integer and save in r0z
    parse_bigint(q0wStr, r0w, primeWords);//convert the hex string from q0zStr into a big integer and save in r0z
    gfp_cr_negate(r0w, r0w, primeNumber);//negate W1 and save it in q0w

    parse_bigint(q_tempStr, qx_temp, primeWords);//convert the hex string from q_tempStr into a big integer and save in qx_temp
    parse_bigint(q_tempStr, qy_temp, primeWords);//convert the hex string from q_tempStr into a big integer and save in qy_temp
    parse_bigint(q_tempStr, qz_temp, primeWords);//convert the hex string from q_tempStr into a big integer and save in qz_temp

    parse_bigint( RSquared, rsquared, primeWords); //convert the hex string from RSquared into a big integer and save in rsquared

    // Perform scalar Multiplication using Right-to-Left Binary Scalar Multiplication
    RightToLeftBinaryScalarMultiply(q0x, q0y, q0z, r0x, r0y, r0z, r0w, qx_temp, qy_temp, qz_temp, kBSrc);

    return 0;

}

//
// End of File
//
