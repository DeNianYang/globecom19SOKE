# globecom19SOKE
This repository consists of the implementation of the paper published in IEEE Globecom 2019: "Optimizing Social-Topic Engagement on Social Network and Knowledge Graph"

## Usage
### Command to make file
```
make
```

### Command to execute cal
```
./cal network n_thread
```

### network files include:

	network_friend

		format: u1 u2

	network_potential

		format: u1 u2 weight

	network_preference graph

		format: user item weight

	network_relevance

		format: user item topic weight

	network_query

		format: a topic per line

	network_baseline

		format: method n m h time user_set_size item_set_size
			a user per line
			an item per line

### Command to execute rel
```
./rel network alpha beta gamma lambda l_walk n_walk
```
	- alpha = Pr(u->u | u)
	- beta = Pr(u->u | i)
	- gamma = Pr(i->t | u)
	- lambda = Pr(i->t | i)
	- l_walk: maximum length of a walk
	- n_walk: number of random walks for calculating a personalized relevance score

### Suggested values of parameters
	- alpha = 0.2
	- beta = 0.6
	- gamma = 0.5
	- lambda = 0.5
	- l_walk = 10
	- n_walk = 100

## Citation
If you find this work or code is helpful in your research, please cite our work:
```
@INPROCEEDINGS{9013546,
  author={Teng, Ya-Wen and Shi, Yishuo and Tsai, Jui-Yi and Shuai, Hong-Han and Tai, Chih-Hua and Yang, De-Nian},
  booktitle={2019 IEEE Global Communications Conference (GLOBECOM)}, 
  title={Optimizing Social-Topic Engagement on Social Network and Knowledge Graph}, 
  year={2019},
  pages={1-6},
  doi={10.1109/GLOBECOM38437.2019.9013546}}
```
