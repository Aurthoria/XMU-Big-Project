//���ߣ�������
//����������
//����ģʽ
#include <iostream>
#include <fstream>
#include <time.h>
#include <iomanip>
#include <Eigen/Dense>//��������
#include<cstdlib>
#include<ctime>
clock_t start,end;
using namespace Eigen;//ʹ��Eigen�����ռ�
using namespace std;

#define INPUT_NUM 4//�������Ŀ
#define HIDDEN_NUM 14//���ز���Ŀ
#define OUTPUT_NUM 3 //�������Ŀ
#define dataMaxNum 200

string specie1="Iris-setosa";
string specie2="Iris-versicolor";
string specie3="Iris-virginica";
int dataNum=0;
int testNum=0;
MatrixXd data(INPUT_NUM,dataMaxNum);
MatrixXd allOutput(OUTPUT_NUM,dataMaxNum);

MatrixXd testData(INPUT_NUM,dataMaxNum);
MatrixXd testOutput(OUTPUT_NUM,dataMaxNum);

class NeuralNetwork
{
private:

    MatrixXd W1,W2;//ע�⣺�����ڴ˴�ֱ�Ӷ�������С����Ӧ���ڹ��캯��������С
    VectorXd B1,B2;
    VectorXd x_max,x_min;

    //singleton������ģʽ��
    static NeuralNetwork *instance;
    NeuralNetwork()//�˴����ǶԾ����С���г�ʼ��
       :W1(HIDDEN_NUM,INPUT_NUM),W2(OUTPUT_NUM,HIDDEN_NUM),B1(HIDDEN_NUM),B2(OUTPUT_NUM),x_max(INPUT_NUM),x_min(INPUT_NUM)
    {
        //������̬�ֲ��������
        for(int i=0;i<HIDDEN_NUM;i++)
        {
            for(int j=0;j<INPUT_NUM;j++)
            {
                W1(i,j)=gaussrand();
            }
            B1(i)=gaussrand();
        }
        for(int i=0;i<OUTPUT_NUM;i++)
        {
            for(int j=0;j<HIDDEN_NUM;j++)
            {
                W2(i,j)=gaussrand();
            }
            B2(i)=gaussrand();
        }
        for(int i=0;i<INPUT_NUM;i++)
        {
            x_max(i)=x_min(i)=0;//��ʼ��
        }
    }

    bool have_train(void)//�ж��Ƿ���й���Чѵ��
    {
        bool judge=true;

        for(int i=0;i<INPUT_NUM;i++)
        {
            if(x_max(i)==x_min(i))
            {
                judge=false;
                break;
            }
        }
        return judge;
    }

    template <typename Derived>
    void normalization(MatrixBase<Derived>& data,int dataNum)//��һ������
    {
        x_max=data.col(0);x_min=data.col(0);
        for(int i=1;i<dataNum;i++)
        {
            for(int j=0;j<INPUT_NUM;j++)
            {
                x_max(j)=max(x_max(j),data(j,i));
                x_min(j)=min(x_min(j),data(j,i));
            }
        }
        for(int i=0;i<dataNum;i++)
        {
            for(int j=0;j<INPUT_NUM;j++)
            {
                data(j,i)=(data(j,i)-x_min(j))/(x_max(j)-x_min(j));
            }
        }
    }

    double gaussrand()//��˹(��̬)�ֲ�
    {
        static double V1, V2, S;
        static int phase = 0;
        double X;
        if(phase==0)
        {
            do
            {
                double U1 = (double)rand() / RAND_MAX;
                double U2 = (double)rand() / RAND_MAX;
                V1=2*U1-1;
                V2=2*U2-1;
                S=V1*V1+V2*V2;
            }while(S>=1||S==0);
            X=V1*sqrt(-2*log(S)/S);
        }
        else
        {
            X=V2*sqrt(-2*log(S)/S);
        }
        phase=1-phase;
        return X;
    }

    double sigmoid(double x)//�����sigmoid
    {
        return 1/(1+exp(-x));
    }

    double deriv_sigmoid(double x)//�����sigmoid�ĵ���
    {
        double fx=sigmoid(x);
        return fx*(1-fx);
    }


    template <typename Derived>
    void feedforward(MatrixBase<Derived>& x,MatrixBase<Derived>& y)//ǰ��������
    {
        VectorXd h(HIDDEN_NUM);
        h=W1*x+B1;
        for(int i=0;i<HIDDEN_NUM;i++)
        {
            h(i)=sigmoid(h(i));
        }
        y=W2*h+B2;
        for(int i=0;i<OUTPUT_NUM;i++)
        {
            y(i)=sigmoid(y(i));
        }
    }

    template <typename Derived>
    double mse_loss(MatrixBase<Derived>& y_true,MatrixBase<Derived>& y_pred,int dataNum)//��ʧ�������������J=1/2*(Y'-Y)^2
    {
        double sum=0;
        MatrixXd D_y;
        D_y=y_true-y_pred;
        for(int i=0;i<dataNum;i++)
        {
            sum+=0.5*(D_y.col(i)).dot(D_y.col(i));//��������ʧ����
        }
        return sum/dataNum;
    }

public:

    static NeuralNetwork* getinstance()
    {
        if(instance==NULL)
        {
            instance=new NeuralNetwork();
        }
        return instance;
    }

    template <typename Derived>
    void train(MatrixBase<Derived>& data,MatrixBase<Derived>& y_true,int dataNum)//ѵ������
    {
        normalization(data,dataNum);//��һ������
        double learn_rate=0.1;//ѧϰ��(����)��ʼΪ0.1
        double loss=1;//��ʼloss
        double lossed=2;
        int epoch=0;//�������ݼ���ѵ������

        //����Ԥ��ֵ{
        VectorXd h(HIDDEN_NUM);
        VectorXd o(OUTPUT_NUM);
        VectorXd sum_h(HIDDEN_NUM);
        VectorXd sum_o(OUTPUT_NUM);
        VectorXd y_pred(OUTPUT_NUM);//������Ԥ��ֵ
        MatrixXd y_preds(OUTPUT_NUM,dataMaxNum);//��������Ԥ��ֵ
        //}

        //��ʧ����J��y_predƫ��
        VectorXd d_J_d_ypred(OUTPUT_NUM);

        //�������Ԫ{
        MatrixXd d_ypred_d_w2(OUTPUT_NUM,HIDDEN_NUM);
        VectorXd d_ypred_d_b2(OUTPUT_NUM);
        MatrixXd d_ypred_d_h(OUTPUT_NUM,HIDDEN_NUM);
        //}

        //���ز���Ԫ{
        MatrixXd d_h_d_w1(HIDDEN_NUM,INPUT_NUM);
        VectorXd d_h_d_b1(HIDDEN_NUM);
        //}

        //����J(X,Y,W1,B1,W2,B2)��W1,B1,W2,B2��ƫ��ֵ{
        VectorXd d_J_d_h(HIDDEN_NUM);
        MatrixXd d_J_d_w1(HIDDEN_NUM,INPUT_NUM);
        VectorXd d_J_d_b1(HIDDEN_NUM);
        MatrixXd d_J_d_w2(OUTPUT_NUM,HIDDEN_NUM);
        VectorXd d_J_d_b2(OUTPUT_NUM);
        //}

        while(abs(lossed-loss)>=0.00001)
        {

            for(int k=0;k<dataNum;k++)
            {
            //**************����Ԥ��ֵy_pred***********************************************************
                sum_h=W1*data.col(k)+B1;
                for(int i=0;i<HIDDEN_NUM;i++)
                {
                    h(i)=sigmoid(sum_h(i));
                }//h=sigmoid(sum_h)=sig_moid(W1*x+B1)

                sum_o=W2*h+B2;
                for(int i=0;i<OUTPUT_NUM;i++)
                {
                    o(i)=sigmoid(sum_o(i));
                }//o=sigmoid(sum_o)=sig_moid(W2*h+B2)

                y_pred=o;//��ǰ�������
            //*******************************************************************************************

            //*************��ʧ����J��y_pred��ƫ��*******************************************************
            //��������ʧ����J=0.5*(y_pred-y_true)^2,����d_J_d_ypred=y_pred-y_true
                d_J_d_ypred=y_pred-y_true.col(k);
            //*******************************************************************************************


            //*************�������ԪOUTPUT************************************************************
            //y_pred=o=sigmoid(sum_o)=sig_moid(W2*h+B2),����d_ypred_d_h=sigmoid��(sum_o)*W2
            //d_y_pred_d_w2=sigmoid��(sum_o)*h,d_y_pred_d_b2=sigmoid��(sum_o)

                for(int i=0;i<OUTPUT_NUM;i++)
                {
                    sum_o(i)=deriv_sigmoid(sum_o(i));
                }//��sum_oӳ��Ϊsigmoid'(sum_o)

                for(int i=0;i<OUTPUT_NUM;i++)
                {
                    d_ypred_d_h.row(i)=sum_o(i)*W2.row(i);
                    d_ypred_d_w2.row(i)=sum_o(i)*(h.transpose());
                }//d_ypred_d_h=sigmoid��(sum_o)*W2,d_y_pred_d_w2=sigmoid��(sum_o)*h

                d_ypred_d_b2=sum_o;//d_y_pred_d_b2=sigmoid��(sum_o)
            //********************************************************************************************

            //*************���ز���ԪHIDDEN*************************************************************
            //h=sigmoid(sum_h)=sigmoid(W1*x+B1),����d_h_d_w1=sigmoid'(sum_h)*x
            //d_h_d_b1=sigmoid'(sum_h)

                for(int i=0;i<HIDDEN_NUM;i++)
                {
                    sum_h(i)=deriv_sigmoid(sum_h(i));
                }//��sum_hӳ��Ϊsigmoid'(sum_h)

                for(int i=0;i<HIDDEN_NUM;i++)
                {
                    d_h_d_w1.row(i)=sum_h(i)*(data.col(k)).transpose();
                }//d_h_d_w1=sigmoid'(sum_h)*x

                d_h_d_b1=sum_h;//d_h_d_b1=sigmoid'(sum_h)
             //*******************************************************************************************

             //************����J(X,Y,W1,B1,W2,B2)��W1,B1,W2,B2��ƫ��ֵ************************************
                d_J_d_h=d_ypred_d_h.transpose()*d_J_d_ypred;//(a^T*B)^T=B^T*a
                for(int i=0;i<HIDDEN_NUM;i++)
                {
                    d_J_d_w1.row(i)=d_J_d_h(i)*d_h_d_w1.row(i);//��Ӧ�����
                }
                d_J_d_b1=(d_J_d_h.array()*d_h_d_b1.array()).matrix();//ʵ�ֶ�ӦԪ�����

                for(int i=0;i<OUTPUT_NUM;i++)
                {
                    d_J_d_w2.row(i)=d_J_d_ypred(i)*d_ypred_d_w2.row(i);//��Ӧ�����
                }
                d_J_d_b2=(d_J_d_ypred.array()*d_ypred_d_b2.array()).matrix();//ʵ�ֶ�ӦԪ�����
             //********************************************************************************************

             //***********���򴫲�������������*******************************************************
                W1=W1-learn_rate*d_J_d_w1;
                B1=B1-learn_rate*d_J_d_b1;
                W2=W2-learn_rate*d_J_d_w2;
                B2=B2-learn_rate*d_J_d_b2;
             //********************************************************************************************

            }

            //*******ÿʮ�ָ���һ����ʧ����ֵ(��*******************************************************
            if(epoch%10==0)
            {
                lossed=loss;
                for(int i=0;i<dataNum;i++)
                {
                    VectorXd x(INPUT_NUM);
                    VectorXd y(OUTPUT_NUM);
                    x=data.col(i);
                    feedforward(x,y);
                    y_preds.col(i)=y;
                }
                loss=mse_loss(y_true,y_preds,dataNum);
                //printf("Epoch %d loss %lf\n",epoch,loss);
                printf("%d  %lf\n",epoch,loss);

                learn_rate=exp(-epoch*0.0004);//ѧϰ�ʺ���
            }
            //*********************************************************************************************

            epoch++;

        }
    }

    template <typename Derived>
    void getOutput(MatrixBase<Derived>& input,MatrixBase<Derived>& y)
    {
        if(!have_train())
        {
            cout<<"No train,no gain."<<endl;
            return;
        }
        for(int j=0;j<INPUT_NUM;j++)
        {
            input(j)=(input(j)-x_min(j))/(x_max(j)-x_min(j));
        }
        VectorXd x(INPUT_NUM);x=input;
        VectorXd h(HIDDEN_NUM);
        for(int i=0;i<INPUT_NUM;i++)
        h=W1*x+B1;
        for(int i=0;i<HIDDEN_NUM;i++)
        {
            h(i)=sigmoid(h(i));
        }
        y=W2*h+B2;
        for(int i=0;i<OUTPUT_NUM;i++)
        {
            y(i)=sigmoid(y(i));
        }
    }

    template <typename Derived>
    void decode(MatrixBase<Derived>& Y,MatrixBase<Derived>& output)
    {
        VectorXd code1(3);code1<<1,0,0;
        VectorXd code2(3);code2<<0,1,0;
        VectorXd code3(3);code3<<0,0,1;
        bool s1=true,s2=true,s3=true;
        VectorXd y(OUTPUT_NUM);
        int MaxI=0;
        for(int i=1;i<OUTPUT_NUM;i++)
        {
            if(Y(i)>Y(MaxI)) MaxI=i;
        }
        y(MaxI)=1;
        for(int i=0;i<OUTPUT_NUM;i++)
        {
            if(i==MaxI) continue;
            y(i)=0;
        }
        for(int i=0;i<OUTPUT_NUM;i++)
        {
            if(y(i)!=code1(i)) s1=false;
            if(y(i)!=code2(i)) s2=false;
            if(y(i)!=code3(i)) s3=false;
        }
        if(s1&&!s2&&!s3)
        {
            cout<<specie1<<endl;
            output=code1;
        }
        else if(!s1&&s2&&!s3)
        {
            cout<<specie2<<endl;
            output=code2;
        }
        else if(!s1&&!s2&&s3)
        {
            cout<<specie3<<endl;
            output=code3;
        }
        else
        {
            cout<<"Be unable to identify!"<<endl;
        }
    }

    void print(void)//��ӡ������
    {
        cout<<"W1:"<<endl;
        cout<<W1<<endl;
        cout<<"B1:"<<endl;
        cout<<B1.transpose()<<endl;
        cout<<"W2:"<<endl;
        cout<<W2<<endl;
        cout<<"B2:"<<endl;
        cout<<B2.transpose()<<endl;
    }

    ~NeuralNetwork()//��������
    {
        cout<<"\nNeuralNetwork has been destructed!"<<endl;
    }

};
NeuralNetwork* NeuralNetwork::instance=NULL;//��ʼ����̬��Ա

int main()
{
    srand(time(NULL));//���������

    VectorXd code1(3);code1<<1,0,0;
    VectorXd code2(3);code2<<0,1,0;
    VectorXd code3(3);code3<<0,0,1;

    //�����ļ�����
    ifstream infile;
    infile.open("iris.data");
    cout << "Reading from the file:iris.data" << endl;
    char comma;
    string s;
    while(infile>>data(0,dataNum)>>comma>>data(1,dataNum)>>comma>>data(2,dataNum)>>comma>>data(3,dataNum)>>comma>>s&&dataNum<dataMaxNum)
    {
        if(s==specie1) allOutput.col(dataNum)=code1;
        if(s==specie2) allOutput.col(dataNum)=code2;
        if(s==specie3) allOutput.col(dataNum)=code3;
        dataNum++;
    }
    infile.close();

    infile.open("test.data");
    cout << "Reading from the file:test.data" << endl;
    while(infile>>testData(0,testNum)>>comma>>testData(1,testNum)>>comma>>testData(2,testNum)>>comma>>testData(3,testNum)>>comma>>s&&testNum<dataMaxNum)
    {
        if(s==specie1) testOutput.col(testNum)=code1;
        if(s==specie2) testOutput.col(testNum)=code2;
        if(s==specie3) testOutput.col(testNum)=code3;
        testNum++;
    }
    infile.close();

    //��ʼʵ��һ��������
    NeuralNetwork* network=NeuralNetwork::getinstance();

    start=clock();
    network->train(data,allOutput,dataNum);
    end=clock();		//���������ʱ
	double endtime=(double)(end-start)/CLOCKS_PER_SEC;
	cout<<"Total time:"<<endtime<<endl;		//sΪ��λ
	network->print();//�������

	int matchNum=0;
    MatrixXd y_preds(OUTPUT_NUM,dataMaxNum);
    VectorXd x(INPUT_NUM),y(OUTPUT_NUM);
    for(int i=0;i<testNum;i++)
    {
        x=testData.col(i);
        network->getOutput(x,y);
        y_preds.col(i)=y;
    }
    VectorXd pr(OUTPUT_NUM);
    VectorXd output(OUTPUT_NUM);
    for(int i=0;i<testNum;i++)
    {
        for(int j=0;j<OUTPUT_NUM;j++)
        {
            pr(j)=y_preds(j,i);
        }
        network->decode(pr,output);
        bool flag=true;
        for(int j=0;j<OUTPUT_NUM;j++)
        {
            if(output(j)!=testOutput(j,i))
            {
                flag=false;
                break;
            }
        }
        if(flag==true) matchNum++;
    }//������Լ����Ч��
    cout<<"���Լ�ƥ���ʣ�"<<double(matchNum)/testNum*100<<"%"<<endl;

    //�������ѵ���������ʶ��ʵ��
    VectorXd a(INPUT_NUM),b(INPUT_NUM),c(OUTPUT_NUM);
    a<<5.0,3.5,1.3,0.3;
    b<<7,7,7,7;
    network->getOutput(a,c);
    cout<<"\na���Ԥ��ֵ"<<endl;
    network->decode(c,output);
    network->getOutput(b,c);
    cout<<"b���Ԥ��ֵ"<<endl;
    network->decode(c,output);

    delete network;

    return 0;
}
