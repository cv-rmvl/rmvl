基于 TOPSIS 模型的熵权法 {#tutorial_common_ew_topsis}
============

@author RoboMaster Vision Community
@date 2023/01/11

@prev_tutorial{tutorial_common_ra_heap}

@next_tutorial{tutorial_common_fitsine}

@tableofcontents

------

相关类 rm::EwTopsis

### 熵权法介绍

熵权法是一种客观赋权方法，根据各指标的数据的分散程度，利用信息熵计算出各指标的熵权，再根据各指标对熵权进行一定的修正，从而得到较为客观的指标权重。

有 \f$m\f$ 个样本，每个样本都拥有 \f$n\f$ 个指标，那么可以构成一个 \f$R'_{m\times n}\f$ 样本指标矩阵。

### 熵权法公式

首先计算标准化指标：\f$R\f$

\f[
R_{i,j}=\begin{cases}
\frac{R_{i,j}'-\min\limits_iR_{i,j}'}{\max\limits_iR_{i,j}'-\min\limits_iR_{i,j}'}&j\text{为正指标} \\
\frac{\max\limits_iR_{i,j}'-R_{i,j}'}{\max\limits_iR_{i,j}'-\min\limits_iR_{i,j}'}&j\text{为负指标}
\end{cases}
\tag{1}
\f]

计算样本值占指标的比重：\f$P\f$

\f[
P_{i,j}=\frac{R_{i,j}}{\sum\limits_{i=1}^m R_{i,j}}\tag{2}
\f]

计算每个指标的熵值：\f$H\f$

\f[
H_j=-\frac{1}{\ln m}\sum\limits_{i=1}^m P_{i,j} \ln{P_{i,j}}\tag{3}
\f]

计算每个指标的熵权：\f$w\f$

\f[
w_j=\frac{1-H_j}{\sum\limits_{j=1}^n(1-H_j)}=\frac{1-H_j}{n-\sum\limits_{j=1}^nH_j}\tag{4}
\f]

最终获得每个样本的综合指标：\f$S\f$

\f[
S_i=\sum\limits_{j=1}^nw_jR_{i,j}'\tag{5}
\f]
