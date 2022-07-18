/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "vsss.h"

using std::vector;
using safeheron::bignum::BN;
using safeheron::curve::CurvePoint;

namespace safeheron{
namespace sss {
namespace vsss {

void MakeShares(vector<Point> &shares, const BN &secret, int threshold, const vector<BN> &shareIndexs, const BN &prime) {
    Polynomial poly = Polynomial::CreateRandomPolynomial(secret, threshold, prime);
    poly.GetPoints(shares, shareIndexs);
}

void MakeSharesWithCommits(vector<Point> &shares, vector<CurvePoint> &commits, const BN &secret, int threshold,
                                 const vector<BN> &shareIndexs, const BN &prime, const CurvePoint &g) {
    Polynomial poly = Polynomial::CreateRandomPolynomial(secret, threshold, prime);
    poly.GetPoints(shares, shareIndexs);
    poly.GetCommits(commits, g);
}

void MakeSharesWithCommitsAndCoes(vector<Point> &shares, vector<CurvePoint> &commits, const BN &secret, int threshold,
                                 const vector<BN> &shareIndexs, const vector<BN> &coeArray, const BN &prime, const CurvePoint &g) {
    assert(coeArray.size() + 1 == threshold);
    Polynomial poly(secret, coeArray, prime);
    poly.GetPoints(shares, shareIndexs);
    poly.GetCommits(commits, g);
}

bool VerifyShare(const vector<CurvePoint> &commits, const BN &shareIndex, const BN &share, const CurvePoint &g, const BN &prime) {
    return Polynomial::VerifyCommits(commits, shareIndex, share, g, prime);
}

void RecoverSecret(BN &secret, const vector<Point> &shares, const BN &prime) {
    BN x = BN::ZERO;
    Polynomial::LagrangeInterpolate(secret, x, shares, prime);
}

}
}
}
