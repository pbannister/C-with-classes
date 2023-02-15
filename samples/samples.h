#pragma once
//
//  Generate a bunch of random-ish strings.
//

class samples_base_o {
public:
    virtual const char* sample_pick() const = 0;
    void samples_print(unsigned);
};

class simple_sample_o : public samples_base_o {
protected:
    char* p_sample = 0;
    unsigned n_sample = 0;

public:
    ~simple_sample_o();

public:
    bool sample_fill(unsigned);

public:
    const char* sample_pick() const override;
};

class bucket_of_samples_o : public samples_base_o {
protected:
    char* p_bucket = 0;
    unsigned n_space = 0;

public:
    ~bucket_of_samples_o();

public:
    bool bucket_fill(unsigned n = 80, unsigned q = 0);

public:
    const char* sample_pick() const override;
};
