#!/bin/bash
compiler_src_dir=$(realpath $(dirname "$0")/../../src)
test_src_dir=$(realpath $(dirname "$0")/../src)
func_testcase_dir=$(realpath $(dirname "$0")/../functional)
build_dir=$(realpath $(dirname "$0")/../../build)
libsysy=$(realpath $(dirname "$0")/../../)/libsysy.a

compile() {
	cd $build_dir

	if [ $1 = "ast" ]; then
		clang++-10 $test_src_dir/test-ast.cpp $compiler_src_dir/*.cpp -o $build_dir/test-$1
	fi
}


asm() {
	#ast func_name
	#echo $2
	#compile ast

	if [ -z $1 ]; then
		# all test
		test_file_list=`realpath --relative-base=$func_testcase_dir $func_testcase_dir/*.sy`
		for x in $test_file_list
		do
			test_name=${x%.sy}
			echo -n $test_name
			echo -n ": "
			cp $func_testcase_dir/$test_name.* $build_dir
			$build_dir/test-asm -S $func_testcase_dir/$test_name.sy -o $build_dir/$test_name.s
			if [ $? != 0 ]; then
				echo fail; exit
			fi
			arm-linux-gnueabihf-gcc -march=armv7-a $build_dir/$test_name.s $libsysy -static -o $build_dir/$test_name
			if [ $? != 0 ]; then
				echo "fail to link"; exit
			fi
			if [ -f $func_testcase_dir/$test_name.in ]; then
				qemu-arm -B 0x100000000 $build_dir/$test_name < $func_testcase_dir/$test_name.in > $build_dir/$test_name.out
			else
				qemu-arm -B 0x100000000 $build_dir/$test_name > $build_dir/$test_name.out
			fi
			echo -e \\n$? >> $build_dir/$test_name.out
			diff -Bb  $build_dir/$test_name.out $func_testcase_dir/$test_name.out > /dev/null 2>/dev/null
			if [ $? == 0 ]; then
				echo pass; mv $build_dir/$test_name.s $build_dir/build/;rm $build_dir/$test_name*
			else
				echo fail;\
				echo "Expect:";\
				cat $func_testcase_dir/$test_name.out;\
				echo "Got:";\
				cat $build_dir/$test_name.out;\
				cp $func_testcase_dir/$test_name.sy $build_dir/
				exit
			fi
		done

		# echo $test_name_list
	else
		test_file=`realpath --relative-base=$func_testcase_dir $func_testcase_dir/$1*.sy`
		
		test_name=${test_file%.sy}
		#ref_output_file=$func_testcase_dir/$test_name.out
		
		echo -n $test_name
		echo -n ": "

		#cd $build_dir
		cp $func_testcase_dir/$test_name.* $build_dir/
		time $build_dir/test-asm -S $func_testcase_dir/$test_name.sy -o $build_dir/$test_name.s
		if [ $? != 0 ]; then
			echo fail; exit
		fi
		arm-linux-gnueabihf-gcc -march=armv7-a $build_dir/$test_name.s $libsysy -static -o $build_dir/$test_name
		if [ $? != 0 ]; then
			echo "fail to link"; exit
		fi
		if [ -f $func_testcase_dir/$test_name.in ]; then
			time qemu-arm -B 0x100000000 $build_dir/$test_name < $func_testcase_dir/$test_name.in > $build_dir/$test_name.out
		else
			time qemu-arm -B 0x100000000 $build_dir/$test_name > $build_dir/$test_name.out
		fi
		echo -e \\n$? >> $build_dir/$test_name.out
		diff -Bb $build_dir/$test_name.out $func_testcase_dir/$test_name.out > /dev/null 2>/dev/null
		if [ $? == 0 ]; then
			echo pass; mv $build_dir/$test_name.s $build_dir/build/; rm $build_dir/$test_name*
		else
			echo fail;\
			echo "Expect:";\
			cat $func_testcase_dir/$test_name.out;\
			echo "Got:";\
			cat $build_dir/$test_name.out;\
			cp $func_testcase_dir/$test_name.sy $build_dir/ ;\
			exit
		fi
	fi
}

test_single() {
	test_file=`realpath --relative-base=$func_testcase_dir $func_testcase_dir/$1.sy`	
	test_name=${test_file%.sy}
	#ref_output_file=$func_testcase_dir/$test_name.out
	
	echo -n $test_name
	echo ": "

	#cd $build_dir
	cp $func_testcase_dir/$test_name.* $build_dir/
	$build_dir/test-asm -S $func_testcase_dir/$test_name.sy -o $build_dir/$test_name.s
	if [ $? != 0 ]; then
		echo fail; exit -1
	fi
	arm-linux-gnueabihf-gcc -march=armv7-a $build_dir/$test_name.s $libsysy -static -o $build_dir/$test_name
	if [ $? != 0 ]; then
		echo "fail to link"; exit -1
	fi
	if [ -f $func_testcase_dir/$test_name.in ]; then
		qemu-arm -B 0x100000000 $build_dir/$test_name < $func_testcase_dir/$test_name.in > $build_dir/$test_name.out
	else
		qemu-arm -B 0x100000000 $build_dir/$test_name > $build_dir/$test_name.out
	fi
	echo -e \\n$? >> $build_dir/$test_name.out
	diff -Bb $build_dir/$test_name.out $func_testcase_dir/$test_name.out > /dev/null 2>/dev/null
	if [ $? == 0 ]; then
		echo pass; mkdir -p $build_dir/build/; mv $build_dir/$test_name.s $build_dir/build/; rm $build_dir/$test_name $build_dir/$test_name.*
	else
		echo fail;\
		echo "Expect:";\
		cat $func_testcase_dir/$test_name.out;\
		echo "Got:";\
		cat $build_dir/$test_name.out;\
		cp $func_testcase_dir/$test_name.sy $build_dir/ ;\
		exit -1
	fi
}

main() {
	if [ $1 = 'ast' ]; then
		ast $2
	elif [ $1 = 'ir' ]; then
		ir $2
	elif [ $1 = 'asm' ]; then
		asm $2
	elif [ $1 = 'test_single' ]; then
		test_single $2
	fi
}

main $@
